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

def showFig(df):
    for col in ['delay']:
        fig,(ax) = plt.subplots(figsize=(8,5))
        a_val = 1.
        colors = ['r','b','g']
        circ1 = mpatches.Patch(edgecolor=colors[0],alpha=a_val,linestyle ='-',label='MAP - sim', fill=False)
        circ2 = mpatches.Patch(edgecolor=colors[0],alpha=a_val,linestyle ='--',label='MAP - inf',fill=False)
        circ3 = mpatches.Patch(edgecolor=colors[1],alpha=a_val,linestyle ='-',label='Poisson - sim', fill=False)
        circ4 = mpatches.Patch(edgecolor=colors[1],alpha=a_val,linestyle='--', label='Poisson - inf', fill=False)
        circ5 = mpatches.Patch(edgecolor=colors[2],alpha=a_val,linestyle ='-',label='Onoff - sim', fill=False)
        circ6 = mpatches.Patch(edgecolor=colors[2],alpha=a_val,linestyle='--', label='Onoff - inf', fill=False)
        
        
        for i, c in zip(['MAP','Poisson','Onoff'], colors):
            bins=np.histogram(np.hstack((df[df.tp==i][col+'_sim'].values, 
                                         df[df.tp==i][col+'_inf'].values)), bins=100)[1]  
            plt.hist(df[df.tp==i][col+'_sim'].values, bins, density=True, color=c, histtype='step',  linewidth=1.5);
            plt.hist(df[df.tp==i][col+'_inf'].values, bins, density=True, color=c, histtype='step', linestyle='--', linewidth=1.5);
        ax.legend(handles = [circ1,circ2,circ3,circ4,circ5,circ6],loc=1, fontsize = 14)
        plt.xlabel(col.capitalize()+' (ms)', fontsize = 14)
        plt.ylabel('PDF', fontsize = 14)
        plt.tick_params(labelsize=12)
        plt.savefig("pdf.png", dpi=500)
        
        txt_list = []
        fig,(ax) = plt.subplots(figsize=(8,5))
        for i, c in zip(['MAP','Poisson','Onoff'], colors):
            res=stats.relfreq(df[df.tp==i][col+'_sim'].values, numbins=100)
            x_sim=res.lowerlimit + np.linspace(0, res.binsize*res.frequency.size, res.frequency.size)
            y_sim=np.cumsum(res.frequency)
            plt.plot(x_sim, y_sim,color=c,  linewidth=1.5)
            res=stats.relfreq(df[df.tp==i][col+'_inf'].values, numbins=100)
            x_inf=res.lowerlimit + np.linspace(0, res.binsize*res.frequency.size, res.frequency.size)
            y_inf=np.cumsum(res.frequency)
            plt.plot(x_inf, y_inf,color=c, linestyle='--',   linewidth=1.5)
            b1 = [0]*len(y_sim)
            txt = 'Normalized W1 of {}: {:5.5f}'.format(i, wasserstein_distance(y_sim, y_inf)/wasserstein_distance(y_sim, b1))
            print(txt)
            txt_list.append(txt)
        plt.xlabel('End-to-end latency'+' (ms)', fontsize = 14)
        plt.ylabel('CDF', fontsize = 14)
        ax.legend(handles = [circ1,circ2,circ3,circ4,circ5,circ6],loc=4, fontsize = 14)
        plt.tick_params(labelsize=12)
        txt_plt = ''
        for txt in txt_list:
            txt_plt = txt_plt + txt + '\n'
        plt.text(0, 0.6, txt_plt, fontsize=12)
        plt.savefig("cdf.png", dpi=500)


def mergeTrace():
    result=pd.DataFrame()
    for traffic_pattern in tqdm.tqdm(['onoff','poisson','map']):
        # for filename in  ['rsim1', 'rsim2', 'rsim3', 'rsim4', 'rsim5']:
        for filename in  ['rsim1']:
            t=pd.read_csv('../runs/dcn_fattree_finite_load_trace_q128_w100_t{}_{}/reports_ana/latency_per_flow_merged.csv'.format(traffic_pattern, filename))
            t['delay_sim']=t['latency_sim']
            t['delay_inf']=t['latency_inf']
            # t['fd']=t['path'].apply(lambda x: len(x.split('-'))) 
            # t['jitter_sim']=t.groupby(['src_port', 'path'])['delay_sim'].diff().abs()
            # t['jitter_pred']=t.groupby(['src_port', 'path'])['delay_pred'].diff().abs()
            if traffic_pattern=='map':
                t['tp']='MAP'
            else:
                t['tp']=traffic_pattern.capitalize()
            result=pd.concat([result, t], ignore_index=True)     
    return result 
             
    
result=mergeTrace()
showFig(result.dropna())