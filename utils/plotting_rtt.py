import warnings
warnings.filterwarnings("ignore")
from scipy import stats
import pandas as pd 
import numpy as np 
import matplotlib.pyplot as plt 
import seaborn as sns
sns.set_style('ticks')
import tqdm
import matplotlib.patches as mpatches
import scipy.stats as measures
from scipy.stats import wasserstein_distance
import os, sys

make_plot = True

def showFig(df):
    for col in ['rtt']:

        if make_plot is True :
           fig,(ax) = plt.subplots(figsize=(8,5))
           a_val = 1.
        colors = ['r','b','g']

        if make_plot is True :
           circ1 = mpatches.Patch(edgecolor=colors[0],alpha=a_val,linestyle ='-',label='Poisson(DQN) - sim', fill=False)
           circ2 = mpatches.Patch(edgecolor=colors[1],alpha=a_val,linestyle ='--',label='Poisson(DQN) - ME',fill=False)
           circ3 = mpatches.Patch(edgecolor=colors[2],alpha=a_val,linestyle ='--',label='Poisson(DQN) - ME+RT', fill=False)
           
           

           bins=np.histogram(np.hstack((df[df.tp=='all-to-all'][col+'_sim'].values, 
                                           df[df.tp=='all-to-all'][col+'_inf'].values,
                                           df[df.tp=='all-to-all'][col+'_correction_inf'].values,)), bins=100)[1]  
           plt.hist(df[df.tp=='all-to-all'][col+'_sim'].values, bins, density=True, color=colors[0], histtype='step',  linewidth=1.5);
           plt.hist(df[df.tp=='all-to-all'][col+'_inf'].values, bins, density=True, color=colors[1], histtype='step', linestyle='--', linewidth=1.5);
           plt.hist(df[df.tp=='all-to-all'][col+'_correction_inf'].values, bins, density=True, color=colors[2], histtype='step', linestyle='--', linewidth=1.5);
           ax.legend(handles = [circ1,circ2,circ3],loc=1, fontsize = 14)
           plt.xlabel(col.capitalize()+' (ms)', fontsize = 14)
           plt.ylabel('PDF', fontsize = 14)
           plt.tick_params(labelsize=12)
           plt.savefig("rtt-pdf.png", dpi=500)
        
        txt_list = []
        if make_plot is True :
           fig,(ax) = plt.subplots(figsize=(8,5))
        res=stats.relfreq(df[df.tp=='all-to-all'][col+'_sim'].values, numbins=100)
        x_sim=res.lowerlimit + np.linspace(0, res.binsize*res.frequency.size, res.frequency.size)
        y_sim=np.cumsum(res.frequency)
        if make_plot is True :
           plt.plot(x_sim, y_sim,color=colors[0],  linewidth=1.5)

        res=stats.relfreq(df[df.tp=='all-to-all'][col+'_inf'].values, numbins=100)
        x_inf=res.lowerlimit + np.linspace(0, res.binsize*res.frequency.size, res.frequency.size)
        y_inf=np.cumsum(res.frequency)
        if make_plot is True :
           plt.plot(x_inf, y_inf,color=colors[1], linestyle='--',   linewidth=1.5)

        res=stats.relfreq(df[df.tp=='all-to-all'][col+'_correction_inf'].values, numbins=100)
        x_correction_inf=res.lowerlimit + np.linspace(0, res.binsize*res.frequency.size, res.frequency.size)
        y_correction_inf=np.cumsum(res.frequency)
        if make_plot is True :
           plt.plot(x_correction_inf, y_correction_inf,color=colors[2], linestyle='--',   linewidth=1.5)

        b1 = [0]*len(x_sim)
        txt = 'avgRTT(w1) of ME: {:5.5f}'.format(wasserstein_distance(x_sim, x_inf)/wasserstein_distance(x_sim, b1))
        print(txt)
        txt_list.append(txt)

        txt = 'avgRTT(w1) of ME+RT: {:5.5f}'.format(wasserstein_distance(x_sim, x_correction_inf)/wasserstein_distance(x_sim, b1))
        print(txt)
        txt_list.append(txt)

        txt = 'p99RTT(w1) of ME: {:5.5f}'.format(np.abs(np.percentile(x_sim, 99) - np.percentile(x_inf, 99))/(np.percentile(x_sim, 99) - np.percentile(b1, 99)))
        print(txt)
        txt_list.append(txt)

        txt = 'p99RTT(w1) of ME+RT: {:5.5f}'.format(np.abs(np.percentile(x_sim, 99) - np.percentile(x_correction_inf, 99))/(np.percentile(x_sim, 99) - np.percentile(b1, 99)))
        print(txt)
        txt_list.append(txt)


        txt = 'MAPE of ME: {:5.2f}%'.format(np.mean(df[df.tp=='all-to-all']['abs_pct_error_rtt_inf']))
        print(txt)
        txt_list.append(txt)

        txt = 'MAPE of ME+RT: {:5.2f}%'.format(np.mean(df[df.tp=='all-to-all']['abs_pct_error_rtt_correction_inf']))
        print(txt)
        txt_list.append(txt)

        if make_plot is True :
           plt.xlabel('RTT'+' (ms)', fontsize = 14)
           plt.ylabel('CDF', fontsize = 14)
           ax.legend(handles = [circ1,circ2,circ3],loc=4, fontsize = 14)
           plt.tick_params(labelsize=12)
           txt_plt = ''
           for txt in txt_list:
               txt_plt = txt_plt + txt + '\n'
           plt.text(0.11, 0.6, txt_plt, fontsize=12)
           # plt.title('FatTree16, all-to-all, TCP, link utilization = {:5.2f}'.format(1*15*554/500/100), fontsize = 14)
           plt.title('FatTree16, Poisson(DQN) rsim5, TCP, link utilization = {:5.2f}'.format(0.06), fontsize = 14)
           plt.savefig("rtt_cdf_fattree16_alltoall_tcp_dqn_poisson_rsim5.png", dpi=500)


def mergeTrace():
    result=pd.DataFrame()
    for traffic_pattern in tqdm.tqdm(['all-to-all']):
        for datarate in  ['5']:
            # t=pd.read_csv('../runs/dcn_fattree_finite_large_v3_q128_w1000_tpkt_500_r_{}_16x16alltoall/reports_ana/latency_per_flow_merged.csv.rtt'.format(datarate))
            # t=pd.read_csv('../runs/dcn_fattree_finite_load_trace_q128_w1000_rand1_tpoisson_rsim{}_tcp/reports_ana/latency_per_flow_merged.csv.rtt'.format(datarate))
            t=pd.read_csv(sys.argv[1])
            print(os.path.basename(os.path.dirname(os.path.dirname(sys.argv[1]))))
            if traffic_pattern=='map':
                t['tp']='MAP'
            else:
                t['tp']=traffic_pattern
            result=pd.concat([result, t], ignore_index=True)     
    return result 
             
    
result=mergeTrace()
# showFig(result.dropna())
showFig(result)
