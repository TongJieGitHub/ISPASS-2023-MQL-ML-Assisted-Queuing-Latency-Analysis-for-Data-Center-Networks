import os
import sys
import glob
import pandas as pd
import numpy as np
import shutil

def merge_header(tag, seed, csv) :
    ''' This function attaches the header for a result CSV generated from simulation
        For the end_to_end_latency.csv file: "headers/end_to_end_latency_header.txt"
        For the flow_queue_log.csv     file: "headers/flow_queue_log_header.txt"
        ## TODO: Integrate the header information directly into simulation
        ## rather than attaching it later
        ## TODO: Open the file stream once in the CSV generation in flow-log and mimic-log 
        ## rather than open it for every timestamp
        ########################################################################################
    '''

    ## Identify the corresponding header file
    if 'end_to_end' in csv :
        header = "headers/end_to_end_latency_header.txt"
    else :
        header = "headers/flow_queue_log_header.txt"
    ## if 'end_to_end' in csv :

    ## Specify path to CSV
    csv = './runs/' + tag + '/reports_sim/' + csv

    ## List of files to merge
    files = [header, csv]

    ## Temporary output file handle
    output_file = open('./runs/' + tag + '/temp_seed' + str(seed) + '.csv', 'w')

    ## Merging files
    for f in files :
        file_handle = open(f, 'r')
        output_file.write(file_handle.read())
        file_handle.close()
    ## for f in files :

    ## Moving temporary CSV back to the original file
    output_file.close()
    shutil.move('runs/' + tag + '/temp_seed' + str(seed) + '.csv', csv)
## def merge_header(tag, queue) :


def merge_latency_per_flow(tag, sim_filename, ana_filename, latency_per_flow_filename) :
    ''' This function combines simulation and analytical reports to generate latency per flow
    '''

    ## Temporarily redirect output to file
    log_file = open('runs/' + tag + '/reports_ana/summary1.rpt', 'w')

    ## Read CSVs
    sim_data = pd.read_csv(sim_filename)
    ana_data = pd.read_csv(ana_filename)

    ## Create dataframes
    sim_df   = pd.DataFrame(sim_data)
    ana_df   = pd.DataFrame(ana_data)

    ## Create a merged dataframe 
    ## Merge based on certain fields in the CSVs
    merged_df = pd.merge(sim_df, ana_df,
                      on=['timestamp', 'flowId', 'client', 'server'],
                      how='inner')

    ## Compute the error between simulation and analytical model latencies
    merged_df['pct_error_latency_inf'] = ((merged_df['latency_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['pct_error_latency_finC'] = ((merged_df['latency_finC'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['pct_error_latency_finR'] = ((merged_df['latency_finR'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    
    merged_df['abs_pct_error_latency_inf'] = ((merged_df['latency_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100
    merged_df['abs_pct_error_latency_finC'] = ((merged_df['latency_finC'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100
    merged_df['abs_pct_error_latency_finR'] = ((merged_df['latency_finR'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100

    merged_df['pct_error_latency_correction_inf'] = ((merged_df['latency_correction_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['abs_pct_error_latency_correction_inf'] = ((merged_df['latency_correction_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100

    ## Store output CSV to file
    merged_df.to_csv(latency_per_flow_filename, index=False)

    df_ack = merged_df[(merged_df['meanPacketSize'] < 70)].copy()
    log_file.write('ACK  end-to-end:\t\tmape_ME=%6.2f \t mape_ME_RT=%6.2f\n'%(np.mean(df_ack['abs_pct_error_latency_inf']), np.mean(df_ack['abs_pct_error_latency_correction_inf'])))
    df_data = merged_df[(merged_df['meanPacketSize'] >= 70)].copy()
    log_file.write('DATA end-to-end:\t\tmape_ME=%6.2f \t mape_ME_RT=%6.2f\n'%(np.mean(df_data['abs_pct_error_latency_inf']), np.mean(df_data['abs_pct_error_latency_correction_inf'])))
    log_file.write("Overall\tmape_ME=%6.2f\n"%(np.mean(merged_df['abs_pct_error_latency_inf'])))
    log_file.write("Overall\tmape_ME_RT=%6.2f\n"%(np.mean(merged_df['abs_pct_error_latency_correction_inf'])))

    df_rtt = pd.merge(df_data, df_ack,
                     left_on=['timestamp', 'client', 'server'], right_on=['timestamp', 'server', 'client'],
                     how='inner')
    df_rtt['rtt_sim'] = df_rtt['latency_sim_x'] + df_rtt['latency_sim_y']
    df_rtt['rtt_inf'] = df_rtt['latency_inf_x'] + df_rtt['latency_inf_y']
    df_rtt['rtt_correction_inf'] = df_rtt['latency_correction_inf_x'] + df_rtt['latency_correction_inf_y']
    df_rtt['abs_pct_error_rtt_inf'] = ((df_rtt['rtt_inf'] - df_rtt['rtt_sim']) / df_rtt['rtt_sim']).abs() * 100
    df_rtt['abs_pct_error_rtt_correction_inf'] = ((df_rtt['rtt_correction_inf'] - df_rtt['rtt_sim']) / df_rtt['rtt_sim']).abs() * 100
    log_file.write("RTT\tmape_rtt_ME=%6.2f\n"%(np.mean(df_rtt['abs_pct_error_rtt_inf'])))
    log_file.write("RTT\tmape_rtt_ME_RT=%6.2f\n"%(np.mean(df_rtt['abs_pct_error_rtt_correction_inf'])))
    df_rtt.to_csv(latency_per_flow_filename + '.rtt', index=False)

    ## Close file handle
    log_file.close()
## def merge_latency_per_flow(tag, sim_filename, ana_filename, latency_per_flow_filename, latency_per_queue_filename) :

def merge_latency_per_queue(tag, sim_filename, ana_filename, latency_per_queue_filename) :
    ''' This function combines simulation and analytical reports to generate latency per queue
    '''

    ## Temporarily redirect output to file
    log_file = open('runs/' + tag + '/reports_ana/summary2.rpt', 'w')

    ## Read CSVs
    sim_data = pd.read_csv(sim_filename)
    ana_data = pd.read_csv(ana_filename)

    ## Create dataframes
    sim_df   = pd.DataFrame(sim_data)
    ana_df   = pd.DataFrame(ana_data)

    ## Create a merged dataframe 
    ## Merge based on certain fields in the CSVs
    merged_df = pd.merge(sim_df, ana_df,
                      on=['timestamp', 'flowId', 'client', 'server', 'nodeId', 'queueId'],
                      how='inner')
    merged_df['abs_error_CA_sqr'] = ((merged_df['CA_sqr'] - merged_df['CA_sqr_ana']) / merged_df['CA_sqr']).abs() * 100
    
    merged_df['pct_error_latency_inf'] = ((merged_df['latency_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['pct_error_latency_finC'] = ((merged_df['latency_finC'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['pct_error_latency_finR'] = ((merged_df['latency_finR'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    
    merged_df['abs_pct_error_latency_inf'] = ((merged_df['latency_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100
    merged_df['abs_pct_error_latency_finC'] = ((merged_df['latency_finC'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100
    merged_df['abs_pct_error_latency_finR'] = ((merged_df['latency_finR'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100

    merged_df['queueing_delay_inf'] = merged_df['latency_inf'] - merged_df['link_delay_ana']
    merged_df['correction_sim_inf'] = merged_df['queueing_delay_sim'] - merged_df['queueing_delay_inf']
    merged_df['pct_error_latency_correction_inf'] = ((merged_df['latency_correction_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']) * 100
    merged_df['abs_pct_error_latency_correction_inf'] = ((merged_df['latency_correction_inf'] - merged_df['latency_sim']) / merged_df['latency_sim']).abs() * 100

    merged_df.to_csv(latency_per_queue_filename, index=False)

    queues = ['edge.*up', 'aggr.*up', 'core.*down', 'aggr.*down', 'edge.*down']
    for queue in queues :
      df = merged_df[(merged_df['queue'].str.contains(queue)) & (merged_df['meanPacketSize'] < 70)].copy()
      log_file.write('ACK :\t%s\tmape_ME=%6.2f \t mape_ME_RT=%6.2f\n'%(queue,np.mean(df['abs_pct_error_latency_inf']), np.mean(df['abs_pct_error_latency_correction_inf'])))
    for queue in queues :
      df = merged_df[(merged_df['queue'].str.contains(queue)) & (merged_df['meanPacketSize'] >= 70)].copy()
      log_file.write('DATA:\t%s\tmape_ME=%6.2f \t mape_ME_RT=%6.2f\n'%(queue,np.mean(df['abs_pct_error_latency_inf']), np.mean(df['abs_pct_error_latency_correction_inf'])))

    ## Close file handle
    log_file.close()

## def merge_latency_per_queue(tag, sim_filename, ana_filename, latency_per_flow_filename, latency_per_queue_filename) :

def merge_seeds(tag, merged_tag, random_seeds) :
    ''' This function averages metrics from multiple seeds to create merged reports
        The merged reports feed into the analytical models
    '''

    ## Temporarily redirect output to file
    ## Make directory structure if it does not exist
    os.makedirs(merged_tag, exist_ok=True)
    os.makedirs(merged_tag + '/reports_sim', exist_ok=True)
    os.makedirs(merged_tag + '/reports_ana', exist_ok=True)
    os.makedirs(merged_tag + '/reports_others', exist_ok=True)
    os.makedirs(merged_tag + '/outputs_sim', exist_ok=True)

    ## sys.stdout = open(merged_tag + '/reports_others/log.rpt', 'w')
    log_file = open(merged_tag + '/reports_others/log.rpt', 'w')

    print('[ INFO] Beginning to merge simulation outputs to analytical model.')
    log_file.write('[ INFO] Beginning to merge simulation outputs to analytical model.\n')

    ######################################################################################
    ## First data in the simulation output traces that feed into the analytical model
    ######################################################################################
    ## Iterate over all seeds
    for seed_index, seed in enumerate(random_seeds) :

        ## Set filenames
        sim_output_filename = glob.glob('runs/' + tag + '_s' + str(seed) + '/outputs_sim/mimic*txt')

        ## Exit program if file does not exist
        if not os.path.exists(sim_output_filename[0]) :
            msg = '[ERROR] File ' + sim_output_filename + ' does not exist. Exiting...'
            print(msg)
            log_file.write(msg + '\n')
            exit(0)
        ## if os.path.exists(sim_output_filename) :

        msg = '[ INFO] Processing file - ' + sim_output_filename[0]
        print(msg)
        log_file.write(msg + '\n')

        ## Read the CSV
        sim_output_data = pd.read_csv(sim_output_filename[0], sep=':', header=None)

        ## Get ana_model_input filename
        ana_model_input_filename = os.path.basename(sim_output_filename[0])

        ## Sort output data
        sim_output_data.iloc[:,1] = sim_output_data.iloc[:,1].str.replace('c', '').astype('int')
        sim_output_data.iloc[:,2] = sim_output_data.iloc[:,2].str.replace('s', '').astype('int')
        sim_output_data.iloc[:,3] = sim_output_data.iloc[:,3].str.replace('r', '').astype('float32')
        sim_output_data.iloc[:,4] = sim_output_data.iloc[:,4].str.replace('p', '').astype('float32')
        sim_output_data.iloc[:,5] = sim_output_data.iloc[:,5].str.replace('a', '').astype('float32')
        sim_output_data.iloc[:,6] = sim_output_data.iloc[:,6].str.replace('b', '').astype('float32')
        sim_output_data.iloc[:,7] = sim_output_data.iloc[:,7].str.replace('l', '').astype('float32')
        sim_output_data.iloc[:,8] = sim_output_data.iloc[:,8].str.replace('d', '').astype('float32')

        ## Sort by using the client, server and packet size columns
        ## Organize by ACK packets first and then DATA packets
        ## Split the dataframe, sort and concatenate them again
        ## sim_output_data = sim_output_data.sort_values(by=[4,1,2]).reset_index(drop=True)
        sim_output_data_ack  = sim_output_data[sim_output_data.iloc[:,4] < 70].copy()
        sim_output_data_data = sim_output_data[sim_output_data.iloc[:,4] > 70].copy()
        sim_output_data_ack  = sim_output_data_ack.sort_values(by=[1,2]).reset_index(drop=True)
        sim_output_data_data = sim_output_data_data.sort_values(by=[1,2]).reset_index(drop=True)
        sim_output_data      = pd.concat([sim_output_data_ack, sim_output_data_data], ignore_index=True)

        ## Decode the columns
        dr          = sim_output_data.iloc[:,3]
        packet_size = sim_output_data.iloc[:,4]
        ca2         = sim_output_data.iloc[:,5]
        cs2         = sim_output_data.iloc[:,6]
        latency     = sim_output_data.iloc[:,7]
        cd2         = sim_output_data.iloc[:,8]

        ## Merge columns
        ## Create a new dataframe if we are processing the first seed
        ## Concatenate data if it is not the first seed data
        if seed_index == 0 :
            dr_all          = dr.copy()
            packet_size_all = packet_size.copy()
            ca2_all         = ca2.copy()
            cs2_all         = cs2.copy()
            latency_all     = latency.copy()
            cd2_all         = cd2.copy()
        else :
            dr_all          = pd.concat([dr_all, dr], axis=1)
            packet_size_all = pd.concat([packet_size_all, packet_size], axis=1)
            ca2_all         = pd.concat([ca2_all, ca2], axis=1)
            cs2_all         = pd.concat([cs2_all, cs2], axis=1)
            latency_all     = pd.concat([latency_all, latency], axis=1)
            cd2_all         = pd.concat([cd2_all, cd2], axis=1)
        ## if seed_index == 0 :

    ## for seed_index, seed in enumerate(random_seeds) :

    ## Regenerate flow IDs since we are merging data from multiple random seeds
    sim_output_data.iloc[:, 0] = 'i' + pd.Series(list(range(1, len(sim_output_data)+1, 1))).astype('str')
    sim_output_data.iloc[:, 1] = 'c' + sim_output_data.iloc[:, 1].astype('str')
    sim_output_data.iloc[:, 2] = 's' + sim_output_data.iloc[:, 2].astype('str')

    ## Concatenate the results to create a merged frame
    sim_output_data.iloc[:, 3] = 'r' + dr_all.mean(axis=1).astype('str')
    sim_output_data.iloc[:, 4] = 'p' + packet_size_all.mean(axis=1).astype('str')
    sim_output_data.iloc[:, 5] = 'a' + ca2_all.mean(axis=1).astype('str')
    sim_output_data.iloc[:, 6] = 'b' + cs2_all.mean(axis=1).astype('str')
    sim_output_data.iloc[:, 7] = 'l' + latency_all.mean(axis=1).astype('str')
    sim_output_data.iloc[:, 8] = 'd' + cd2_all.mean(axis=1).astype('str')

    msg = '[ INFO] Merging simulation outputs to analytical model complete.'
    print(msg)
    log_file.write(msg + '\n')
    msg = '[ INFO] Beginning to average simulation reports from multiple seeds.'
    print(msg)
    log_file.write(msg + '\n')

    ######################################################################################
    ## Average data in the flow queue reports and end to end latency reports
    ######################################################################################
    ## Iterate over all seeds
    for seed_index, seed in enumerate(random_seeds) :

        ## Set filenames
        latfilename = 'runs/' + tag + '_s' + str(seed) + '/reports_sim/end_to_end_latency.csv'
        flowlogname = 'runs/' + tag + '_s' + str(seed) + '/reports_sim/flow_queue_log.csv'

        ## Exit program if file does not exist
        if (not os.path.exists(latfilename)) or (not os.path.exists(flowlogname)) :
            msg = '[ERROR] Either of files ' + latfilename + ' or ' + flowlogname + ' do not exist. Exiting...'
            print(msg)
            log_file.write(msg + '\n')
            exit(0)
        ## if (not os.path.exists(latfilename)) or (not os.path.exists(flowlogname)) :

        msg = '[ INFO] Processing file - ' + latfilename
        print(msg)
        log_file.write(msg + '\n')
        msg = '[ INFO] Processing file - ' + flowlogname
        print(msg)
        log_file.write(msg + '\n')

        ## Read CSV
        latfile = pd.read_csv(latfilename)
        flowlog = pd.read_csv(flowlogname)

        ## Sort CSVs
        latfile_data = latfile[latfile['meanPacketSize'] > 70].copy()
        latfile_ack  = latfile[latfile['meanPacketSize'] < 70].copy()
        latfile_data = latfile_data.sort_values(by=['client','server']).reset_index(drop=True)
        latfile_ack  = latfile_ack.sort_values(by=['client','server']).reset_index(drop=True)
        latfile      = pd.concat([latfile_ack, latfile_data], ignore_index=True)
        # latfile = latfile.sort_values(by=['client','server','meanPacketSize']).reset_index(drop=True)

        ##################################################################
        ## More sophisticated processing for flowlog
        ##################################################################
        ## Separate DATA and ACK flows
        flowlog = flowlog.sort_values(by=['client','server','nodeId','queueId']).reset_index(drop=True).copy()
        flowlog_data = flowlog[flowlog['meanPacketSize'] > 70].copy()
        flowlog_ack  = flowlog[flowlog['meanPacketSize'] < 70].copy()

        ## Process for ACK
        ## We assign flow IDs first to ACK and then to DATA, because the other files are sorted by meanPacketSize
        ## So, ACK flows will come first
        flowId = 1
        groups_ack  = flowlog_ack.groupby(by=['client','server']).groups
        groups_data = flowlog_data.groupby(by=['client','server']).groups
        for group in groups_ack :
            flowlog_ack.loc[groups_ack[group], 'flowId'] = flowId
            flowId += 1
            # flowlog_data.loc[groups_data[group], 'flowId'] = flowId
            # flowId += 1
        ## for group in groups :
        ## Next, the DATA flows
        ## groups = flowlog_data.groupby(by=['client','server']).groups
        for group in groups_data :
            flowlog_data.loc[groups_data[group], 'flowId'] = flowId
            flowId += 1
        ## for group in groups :

        # flowlog[(flowlog['meanPacketSize'] < 70) & (flowlog['client'] == 15) & (flowlog['server'] == 3)]

        ## Merge the dataframes again
        flowlog_ack  = flowlog_ack.reset_index(drop=True)
        flowlog_data = flowlog_data.reset_index(drop=True)
        flowlog      = pd.concat([flowlog_ack, flowlog_data], ignore_index=True)

        flowlog = flowlog.sort_values(by=['flowId','client','server','nodeId','queueId'])

        ##################################################################

        ## Merge and average
        ## Create a new dataframe if we are processing the first seed
        ## Accumulate data if we are not processing the first seed
        if seed_index == 0 :
            latfile_all = latfile.copy()
            flowlog_all = flowlog.copy()
        else :
            latfile_all['observedDatarate'] += latfile['observedDatarate']
            latfile_all['CA_sqr']           += latfile['CA_sqr']          
            latfile_all['CS_sqr']           += latfile['CS_sqr']          
            latfile_all['meanPacketSize']   += latfile['meanPacketSize']  
            latfile_all['txPackets']        += latfile['txPackets']       
            latfile_all['txBytes']          += latfile['txBytes']         
            latfile_all['rxPackets']        += latfile['rxPackets']       
            latfile_all['rxBytes']          += latfile['rxBytes']         
            latfile_all['cwnd']             += latfile['cwnd']            
            latfile_all['rtt']              += latfile['rtt']             
            latfile_all['latency_sim']      += latfile['latency_sim']   

            flowlog_all['meanDatarateArrival']    += flowlog['meanDatarateArrival']  
            flowlog_all['meanDatarateDeparture']  += flowlog['meanDatarateDeparture']
            flowlog_all['rho']                    += flowlog['rho']                  
            flowlog_all['CA_sqr']                 += flowlog['CA_sqr']               
            flowlog_all['CD_sqr']                 += flowlog['CD_sqr']               
            flowlog_all['CS_sqr']                 += flowlog['CS_sqr']               
            flowlog_all['meanPacketSize']         += flowlog['meanPacketSize']       
            flowlog_all['txPackets']              += flowlog['txPackets']            
            flowlog_all['txBytes']                += flowlog['txBytes']              
            flowlog_all['rxPackets']              += flowlog['rxPackets']            
            flowlog_all['rxBytes']                += flowlog['rxBytes']              
            flowlog_all['latency_sim']            += flowlog['latency_sim']          
            flowlog_all['link_delay_sim']         += flowlog['link_delay_sim']       
            flowlog_all['queueing_delay_sim']     += flowlog['queueing_delay_sim']   
            flowlog_all['queuePackets']           += flowlog['queuePackets']         
            flowlog_all['queueBytes']             += flowlog['queueBytes']         
        ## if seed_index == 0 :

    ## Average out the values
    latfile_all['observedDatarate'] = latfile_all['observedDatarate'] / len(random_seeds)
    latfile_all['CA_sqr']           = latfile_all['CA_sqr']           / len(random_seeds)
    latfile_all['CS_sqr']           = latfile_all['CS_sqr']           / len(random_seeds)
    latfile_all['meanPacketSize']   = latfile_all['meanPacketSize']   / len(random_seeds)
    latfile_all['txPackets']        = latfile_all['txPackets']        / len(random_seeds)
    latfile_all['txBytes']          = latfile_all['txBytes']          / len(random_seeds)
    latfile_all['rxPackets']        = latfile_all['rxPackets']        / len(random_seeds)
    latfile_all['rxBytes']          = latfile_all['rxBytes']          / len(random_seeds)
    latfile_all['cwnd']             = latfile_all['cwnd']             / len(random_seeds)
    latfile_all['rtt']              = latfile_all['rtt']              / len(random_seeds)
    latfile_all['latency_sim']      = latfile_all['latency_sim']      / len(random_seeds)

    flowlog_all['meanDatarateArrival']    = flowlog_all['meanDatarateArrival']   / len(random_seeds)
    flowlog_all['meanDatarateDeparture']  = flowlog_all['meanDatarateDeparture'] / len(random_seeds)
    flowlog_all['rho']                    = flowlog_all['rho']                   / len(random_seeds)
    flowlog_all['CA_sqr']                 = flowlog_all['CA_sqr']                / len(random_seeds)
    flowlog_all['CD_sqr']                 = flowlog_all['CD_sqr']                / len(random_seeds)
    flowlog_all['CS_sqr']                 = flowlog_all['CS_sqr']                / len(random_seeds)
    flowlog_all['meanPacketSize']         = flowlog_all['meanPacketSize']        / len(random_seeds)
    flowlog_all['txPackets']              = flowlog_all['txPackets']             / len(random_seeds)
    flowlog_all['txBytes']                = flowlog_all['txBytes']               / len(random_seeds)
    flowlog_all['rxPackets']              = flowlog_all['rxPackets']             / len(random_seeds)
    flowlog_all['rxBytes']                = flowlog_all['rxBytes']               / len(random_seeds)
    flowlog_all['latency_sim']            = flowlog_all['latency_sim']           / len(random_seeds)
    flowlog_all['link_delay_sim']         = flowlog_all['link_delay_sim']        / len(random_seeds)
    flowlog_all['queueing_delay_sim']     = flowlog_all['queueing_delay_sim']    / len(random_seeds)
    flowlog_all['queuePackets']           = flowlog_all['queuePackets']          / len(random_seeds)
    flowlog_all['queueBytes']             = flowlog_all['queueBytes']            / len(random_seeds)

    latfile_all['flowId'] = pd.Series(list(range(1, len(latfile_all)+1, 1)))

    ## Write out traffic file
    os.makedirs(merged_tag + '/outputs_sim', exist_ok=True)
    os.makedirs(merged_tag + '/reports_sim', exist_ok=True)
    os.makedirs(merged_tag + '/reports_ana', exist_ok=True)

    sim_output_data.to_csv(merged_tag + '/outputs_sim/' + ana_model_input_filename, header=False, index=False, sep=':')
    latfile_all.to_csv(merged_tag + '/reports_sim/end_to_end_latency.csv', index=False)
    flowlog_all.to_csv(merged_tag + '/reports_sim/flow_queue_log.csv', index=False)

    msg = '[ INFO] Averaging simulation reports from multiple seeds complete.'
    print(msg)
    log_file.write(msg + '\n')

    ## Revert stdout back to display
    ## sys.stdout.close()
    ## sys.stdout = sys.__stdout__

## def merge_seeds(merged_tag) :
