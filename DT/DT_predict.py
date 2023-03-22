def predict_client_up(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    return "GEG1"


def predict_client_up_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    return [1.0, 0.0]


def predict_edge_down(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if rho_total <= 0.576372:
        if data_rate <= 8.467472:
            if rho_total <= 0.487756:
                if CA_sqr <= 0.979721:
                    if data_rate <= 6.354501:
                        if rho <= 0.010733:
                            if CA_sqr <= 0.975046:
                                return "link"
                            else:  # if CA_sqr > 0.975046
                                return "GEG1"
                        else:  # if rho > 0.010733
                            return "link"
                    else:  # if data_rate > 6.354501
                        if rho <= 0.063556:
                            return "GEG1"
                        else:  # if rho > 0.063556
                            if rho <= 0.063636:
                                if rho <= 0.063598:
                                    return "link"
                                else:  # if rho > 0.063598
                                    return "GEG1"
                            else:  # if rho > 0.063636
                                return "link"
                else:  # if CA_sqr > 0.979721
                    if data_rate <= 6.36452:
                        if CA_sqr <= 0.980591:
                            if rho_total <= 0.175312:
                                if rho_total <= 0.026541:
                                    return "link"
                                else:  # if rho_total > 0.026541
                                    return "GEG1"
                            else:  # if rho_total > 0.175312
                                return "link"
                        else:  # if CA_sqr > 0.980591
                            if num_flows <= 2.5:
                                return "link"
                            else:  # if num_flows > 2.5
                                if rho_total <= 0.442409:
                                    return "link"
                                else:  # if rho_total > 0.442409
                                    return "link"
                    else:  # if data_rate > 6.36452
                        if num_flows <= 5.5:
                            if rho_total <= 0.275448:
                                return "link"
                            else:  # if rho_total > 0.275448
                                if rho_total <= 0.297064:
                                    return "link"
                                else:  # if rho_total > 0.297064
                                    return "link"
                        else:  # if num_flows > 5.5
                            return "GEG1"
            else:  # if rho_total > 0.487756
                if rho <= 0.074282:
                    return "link"
                else:  # if rho > 0.074282
                    if data_rate <= 7.453752:
                        return "GEG1"
                    else:  # if data_rate > 7.453752
                        return "link"
        else:  # if data_rate > 8.467472
            if num_flows <= 5.5:
                if CA_sqr <= 0.964789:
                    if CA_sqr <= 0.954121:
                        return "link"
                    else:  # if CA_sqr > 0.954121
                        if CA_sqr <= 0.954938:
                            return "GEG1"
                        else:  # if CA_sqr > 0.954938
                            if CA_sqr <= 0.961485:
                                return "link"
                            else:  # if CA_sqr > 0.961485
                                if CA_sqr <= 0.96191:
                                    return "GEG1"
                                else:  # if CA_sqr > 0.96191
                                    return "link"
                else:  # if CA_sqr > 0.964789
                    if data_rate <= 11.627921:
                        if CA_sqr <= 0.984136:
                            if data_rate <= 8.47045:
                                return "GEG1"
                            else:  # if data_rate > 8.47045
                                if rho_total <= 0.380892:
                                    return "link"
                                else:  # if rho_total > 0.380892
                                    return "link"
                        else:  # if CA_sqr > 0.984136
                            if rho <= 0.094749:
                                return "link"
                            else:  # if rho > 0.094749
                                if rho <= 0.105719:
                                    return "GEG1"
                                else:  # if rho > 0.105719
                                    return "GEG1"
                    else:  # if data_rate > 11.627921
                        if data_rate <= 13.779278:
                            return "GEG1"
                        else:  # if data_rate > 13.779278
                            return "link"
            else:  # if num_flows > 5.5
                if rho_total <= 0.539052:
                    return "GEG1"
                else:  # if rho_total > 0.539052
                    if CA_sqr <= 0.870673:
                        return "GEG1"
                    else:  # if CA_sqr > 0.870673
                        return "link"
    else:  # if rho_total > 0.576372
        if rho <= 0.105899:
            if CA_sqr <= 0.983844:
                return "link"
            else:  # if CA_sqr > 0.983844
                if CA_sqr <= 0.9888:
                    return "GEG1"
                else:  # if CA_sqr > 0.9888
                    return "link"
        else:  # if rho > 0.105899
            return "GEG1"



def predict_edge_down_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if rho_total <= 0.576372:
        if data_rate <= 8.467472:
            if rho_total <= 0.487756:
                if CA_sqr <= 0.979721:
                    if data_rate <= 6.354501:
                        if rho <= 0.010733:
                            if CA_sqr <= 0.975046:
                                return [0.0, 1.0]
                            else:  # if CA_sqr > 0.975046
                                return [1.0, 0.0]
                        else:  # if rho > 0.010733
                            return [0.0, 1.0]
                    else:  # if data_rate > 6.354501
                        if rho <= 0.063556:
                            return [1.0, 0.0]
                        else:  # if rho > 0.063556
                            if rho <= 0.063636:
                                if rho <= 0.063598:
                                    return [0.0, 1.0]
                                else:  # if rho > 0.063598
                                    return [1.0, 0.0]
                            else:  # if rho > 0.063636
                                return [0.0, 1.0]
                else:  # if CA_sqr > 0.979721
                    if data_rate <= 6.36452:
                        if CA_sqr <= 0.980591:
                            if rho_total <= 0.175312:
                                if rho_total <= 0.026541:
                                    return [0.0, 1.0]
                                else:  # if rho_total > 0.026541
                                    return [1.0, 0.0]
                            else:  # if rho_total > 0.175312
                                return [0.0, 1.0]
                        else:  # if CA_sqr > 0.980591
                            if num_flows <= 2.5:
                                return [0.0, 1.0]
                            else:  # if num_flows > 2.5
                                if rho_total <= 0.442409:
                                    return [0.196, 0.804]
                                else:  # if rho_total > 0.442409
                                    return [0.0, 1.0]
                    else:  # if data_rate > 6.36452
                        if num_flows <= 5.5:
                            if rho_total <= 0.275448:
                                return [0.0, 1.0]
                            else:  # if rho_total > 0.275448
                                if rho_total <= 0.297064:
                                    return [0.3333333333333333, 0.6666666666666666]
                                else:  # if rho_total > 0.297064
                                    return [0.0, 1.0]
                        else:  # if num_flows > 5.5
                            return [1.0, 0.0]
            else:  # if rho_total > 0.487756
                if rho <= 0.074282:
                    return [0.0, 1.0]
                else:  # if rho > 0.074282
                    if data_rate <= 7.453752:
                        return [1.0, 0.0]
                    else:  # if data_rate > 7.453752
                        return [0.0, 1.0]
        else:  # if data_rate > 8.467472
            if num_flows <= 5.5:
                if CA_sqr <= 0.964789:
                    if CA_sqr <= 0.954121:
                        return [0.0, 1.0]
                    else:  # if CA_sqr > 0.954121
                        if CA_sqr <= 0.954938:
                            return [1.0, 0.0]
                        else:  # if CA_sqr > 0.954938
                            if CA_sqr <= 0.961485:
                                return [0.0, 1.0]
                            else:  # if CA_sqr > 0.961485
                                if CA_sqr <= 0.96191:
                                    return [1.0, 0.0]
                                else:  # if CA_sqr > 0.96191
                                    return [0.0, 1.0]
                else:  # if CA_sqr > 0.964789
                    if data_rate <= 11.627921:
                        if CA_sqr <= 0.984136:
                            if data_rate <= 8.47045:
                                return [1.0, 0.0]
                            else:  # if data_rate > 8.47045
                                if rho_total <= 0.380892:
                                    return [0.0, 1.0]
                                else:  # if rho_total > 0.380892
                                    return [0.125, 0.875]
                        else:  # if CA_sqr > 0.984136
                            if rho <= 0.094749:
                                return [0.0, 1.0]
                            else:  # if rho > 0.094749
                                if rho <= 0.105719:
                                    return [1.0, 0.0]
                                else:  # if rho > 0.105719
                                    return [0.6666666666666666, 0.3333333333333333]
                    else:  # if data_rate > 11.627921
                        if data_rate <= 13.779278:
                            return [1.0, 0.0]
                        else:  # if data_rate > 13.779278
                            return [0.0, 1.0]
            else:  # if num_flows > 5.5
                if rho_total <= 0.539052:
                    return [1.0, 0.0]
                else:  # if rho_total > 0.539052
                    if CA_sqr <= 0.870673:
                        return [1.0, 0.0]
                    else:  # if CA_sqr > 0.870673
                        return [0.0, 1.0]
    else:  # if rho_total > 0.576372
        if rho <= 0.105899:
            if CA_sqr <= 0.983844:
                return [0.0, 1.0]
            else:  # if CA_sqr > 0.983844
                if CA_sqr <= 0.9888:
                    return [1.0, 0.0]
                else:  # if CA_sqr > 0.9888
                    return [0.0, 1.0]
        else:  # if rho > 0.105899
            return [1.0, 0.0]



def predict_edge_up(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if num_flows <= 9.0:
        if num_flows <= 2.5:
            if rho <= 0.010673:
                if CA_sqr <= 0.984683:
                    return "GEG1"
                else:  # if CA_sqr > 0.984683
                    return "link"
            else:  # if rho > 0.010673
                if rho_total <= 0.052889:
                    if rho <= 0.021395:
                        return "GEG1"
                    else:  # if rho > 0.021395
                        return "link"
                else:  # if rho_total > 0.052889
                    return "GEG1"
        else:  # if num_flows > 2.5
            return "link"
    else:  # if num_flows > 9.0
        if rho_total <= 0.953238:
            return "GEG1"
        else:  # if rho_total > 0.953238
            return "link"




def predict_edge_up_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if num_flows <= 9.0:
        if num_flows <= 2.5:
            if rho <= 0.010673:
                if CA_sqr <= 0.984683:
                    return [1.0, 0.0]
                else:  # if CA_sqr > 0.984683
                    return [0.0, 1.0]
            else:  # if rho > 0.010673
                if rho_total <= 0.052889:
                    if rho <= 0.021395:
                        return [1.0, 0.0]
                    else:  # if rho > 0.021395
                        return [0.0, 1.0]
                else:  # if rho_total > 0.052889
                    return [1.0, 0.0]
        else:  # if num_flows > 2.5
            return [0.0, 1.0]
    else:  # if num_flows > 9.0
        if rho_total <= 0.953238:
            return [1.0, 0.0]
        else:  # if rho_total > 0.953238
            return [0.0, 1.0]




def predict_agg_down(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if rho <= 0.163944:
        if CA_sqr <= 1.023491:
            if data_rate <= 6.315583:
                if rho <= 0.053377:
                    if num_flows <= 2.5:
                        return "link"
                    else:  # if num_flows > 2.5
                        if CA_sqr <= 0.99983:
                            if data_rate <= 5.330953:
                                if CA_sqr <= 0.9824:
                                    return "link"
                                else:  # if CA_sqr > 0.9824
                                    return "link"
                            else:  # if data_rate > 5.330953
                                return "GEG1"
                        else:  # if CA_sqr > 0.99983
                            if rho <= 0.042569:
                                if rho_total <= 0.079492:
                                    return "GEG1"
                                else:  # if rho_total > 0.079492
                                    return "link"
                            else:  # if rho > 0.042569
                                if rho_total <= 0.15944:
                                    return "link"
                                else:  # if rho_total > 0.15944
                                    return "GEG1"
                else:  # if rho > 0.053377
                    if data_rate <= 5.877968:
                        if rho <= 0.056555:
                            return "link"
                        else:  # if rho > 0.056555
                            return "GEG1"
                    else:  # if data_rate > 5.877968
                        return "link"
            else:  # if data_rate > 6.315583
                if num_flows <= 4.5:
                    if num_flows <= 2.5:
                        return "link"
                    else:  # if num_flows > 2.5
                        if CA_sqr <= 0.962668:
                            if rho <= 0.137956:
                                return "link"
                            else:  # if rho > 0.137956
                                if CA_sqr <= 0.93642:
                                    return "link"
                                else:  # if CA_sqr > 0.93642
                                    return "GEG1"
                        else:  # if CA_sqr > 0.962668
                            if data_rate <= 11.655611:
                                if CA_sqr <= 0.978977:
                                    return "link"
                                else:  # if CA_sqr > 0.978977
                                    return "link"
                            else:  # if data_rate > 11.655611
                                if CA_sqr <= 0.965007:
                                    return "GEG1"
                                else:  # if CA_sqr > 0.965007
                                    return "GEG1"
                else:  # if num_flows > 4.5
                    if rho_total <= 0.829381:
                        if rho_total <= 0.412707:
                            if data_rate <= 6.350546:
                                return "GEG1"
                            else:  # if data_rate > 6.350546
                                return "link"
                        else:  # if rho_total > 0.412707
                            return "GEG1"
                    else:  # if rho_total > 0.829381
                        if rho <= 0.08052:
                            if CA_sqr <= 1.021465:
                                return "link"
                            else:  # if CA_sqr > 1.021465
                                return "GEG1"
                        else:  # if rho > 0.08052
                            if CA_sqr <= 0.967006:
                                return "GEG1"
                            else:  # if CA_sqr > 0.967006
                                if data_rate <= 8.43011:
                                    return "GEG1"
                                else:  # if data_rate > 8.43011
                                    return "link"
        else:  # if CA_sqr > 1.023491
            if CA_sqr <= 1.051599:
                if rho <= 0.083762:
                    if rho <= 0.0529:
                        if data_rate <= 4.803304:
                            if rho <= 0.010712:
                                return "GEG1"
                            else:  # if rho > 0.010712
                                if CA_sqr <= 1.03021:
                                    return "link"
                                else:  # if CA_sqr > 1.03021
                                    return "link"
                        else:  # if data_rate > 4.803304
                            return "GEG1"
                    else:  # if rho > 0.0529
                        if rho <= 0.083489:
                            return "link"
                        else:  # if rho > 0.083489
                            if rho <= 0.083633:
                                if rho_total <= 0.834574:
                                    return "link"
                                else:  # if rho_total > 0.834574
                                    return "GEG1"
                            else:  # if rho > 0.083633
                                return "link"
                else:  # if rho > 0.083762
                    return "GEG1"
            else:  # if CA_sqr > 1.051599
                return "link"
    else:  # if rho > 0.163944
        if rho_total <= 0.508256:
            if rho <= 0.16963:
                return "GEG1"
            else:  # if rho > 0.16963
                return "link"
        else:  # if rho_total > 0.508256
            return "GEG1"



def predict_agg_down_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if rho <= 0.163944:
        if CA_sqr <= 1.023491:
            if data_rate <= 6.315583:
                if rho <= 0.053377:
                    if num_flows <= 2.5:
                        return [0.0, 1.0]
                    else:  # if num_flows > 2.5
                        if CA_sqr <= 0.99983:
                            if data_rate <= 5.330953:
                                if CA_sqr <= 0.9824:
                                    return [0.15492957746478872, 0.8450704225352113]
                                else:  # if CA_sqr > 0.9824
                                    return [0.3391304347826087, 0.6608695652173913]
                            else:  # if data_rate > 5.330953
                                return [1.0, 0.0]
                        else:  # if CA_sqr > 0.99983
                            if rho <= 0.042569:
                                if rho_total <= 0.079492:
                                    return [1.0, 0.0]
                                else:  # if rho_total > 0.079492
                                    return [0.4074074074074074, 0.5925925925925926]
                            else:  # if rho > 0.042569
                                if rho_total <= 0.15944:
                                    return [0.0, 1.0]
                                else:  # if rho_total > 0.15944
                                    return [1.0, 0.0]
                else:  # if rho > 0.053377
                    if data_rate <= 5.877968:
                        if rho <= 0.056555:
                            return [0.0, 1.0]
                        else:  # if rho > 0.056555
                            return [1.0, 0.0]
                    else:  # if data_rate > 5.877968
                        return [0.0, 1.0]
            else:  # if data_rate > 6.315583
                if num_flows <= 4.5:
                    if num_flows <= 2.5:
                        return [0.0, 1.0]
                    else:  # if num_flows > 2.5
                        if CA_sqr <= 0.962668:
                            if rho <= 0.137956:
                                return [0.0, 1.0]
                            else:  # if rho > 0.137956
                                if CA_sqr <= 0.93642:
                                    return [0.0, 1.0]
                                else:  # if CA_sqr > 0.93642
                                    return [0.8571428571428571, 0.14285714285714285]
                        else:  # if CA_sqr > 0.962668
                            if data_rate <= 11.655611:
                                if CA_sqr <= 0.978977:
                                    return [0.0, 1.0]
                                else:  # if CA_sqr > 0.978977
                                    return [0.4864864864864865, 0.5135135135135135]
                            else:  # if data_rate > 11.655611
                                if CA_sqr <= 0.965007:
                                    return [0.5, 0.5]
                                else:  # if CA_sqr > 0.965007
                                    return [1.0, 0.0]
                else:  # if num_flows > 4.5
                    if rho_total <= 0.829381:
                        if rho_total <= 0.412707:
                            if data_rate <= 6.350546:
                                return [1.0, 0.0]
                            else:  # if data_rate > 6.350546
                                return [0.0, 1.0]
                        else:  # if rho_total > 0.412707
                            return [1.0, 0.0]
                    else:  # if rho_total > 0.829381
                        if rho <= 0.08052:
                            if CA_sqr <= 1.021465:
                                return [0.0, 1.0]
                            else:  # if CA_sqr > 1.021465
                                return [1.0, 0.0]
                        else:  # if rho > 0.08052
                            if CA_sqr <= 0.967006:
                                return [1.0, 0.0]
                            else:  # if CA_sqr > 0.967006
                                if data_rate <= 8.43011:
                                    return [1.0, 0.0]
                                else:  # if data_rate > 8.43011
                                    return [0.125, 0.875]
        else:  # if CA_sqr > 1.023491
            if CA_sqr <= 1.051599:
                if rho <= 0.083762:
                    if rho <= 0.0529:
                        if data_rate <= 4.803304:
                            if rho <= 0.010712:
                                return [1.0, 0.0]
                            else:  # if rho > 0.010712
                                if CA_sqr <= 1.03021:
                                    return [0.4, 0.6]
                                else:  # if CA_sqr > 1.03021
                                    return [0.0, 1.0]
                        else:  # if data_rate > 4.803304
                            return [1.0, 0.0]
                    else:  # if rho > 0.0529
                        if rho <= 0.083489:
                            return [0.0, 1.0]
                        else:  # if rho > 0.083489
                            if rho <= 0.083633:
                                if rho_total <= 0.834574:
                                    return [0.0, 1.0]
                                else:  # if rho_total > 0.834574
                                    return [1.0, 0.0]
                            else:  # if rho > 0.083633
                                return [0.0, 1.0]
                else:  # if rho > 0.083762
                    return [1.0, 0.0]
            else:  # if CA_sqr > 1.051599
                return [0.0, 1.0]
    else:  # if rho > 0.163944
        if rho_total <= 0.508256:
            if rho <= 0.16963:
                return [1.0, 0.0]
            else:  # if rho > 0.16963
                return [0.0, 1.0]
        else:  # if rho_total > 0.508256
            return [1.0, 0.0]




def predict_agg_up(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if num_flows <= 10.0:
        if data_rate <= 14.509334:
            if CA_sqr <= 0.946244:
                return "link"
            else:  # if CA_sqr > 0.946244
                if rho_total <= 0.42304:
                    if CA_sqr <= 0.977279:
                        if data_rate <= 3.720379:
                            if CA_sqr <= 0.974624:
                                return "GEG1"
                            else:  # if CA_sqr > 0.974624
                                if CA_sqr <= 0.974949:
                                    return "link"
                                else:  # if CA_sqr > 0.974949
                                    return "GEG1"
                        else:  # if data_rate > 3.720379
                            return "link"
                    else:  # if CA_sqr > 0.977279
                        if rho_total <= 0.338521:
                            if data_rate <= 7.954653:
                                if rho_total <= 0.295936:
                                    return "GEG1"
                                else:  # if rho_total > 0.295936
                                    return "GEG1"
                            else:  # if data_rate > 7.954653
                                return "link"
                        else:  # if rho_total > 0.338521
                            return "GEG1"
                else:  # if rho_total > 0.42304
                    return "GEG1"
        else:  # if data_rate > 14.509334
            return "GEG1"
    else:  # if num_flows > 10.0
        return "GEG1"



def predict_agg_up_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    if num_flows <= 10.0:
        if data_rate <= 14.509334:
            if CA_sqr <= 0.946244:
                return [0.0, 1.0]
            else:  # if CA_sqr > 0.946244
                if rho_total <= 0.42304:
                    if CA_sqr <= 0.977279:
                        if data_rate <= 3.720379:
                            if CA_sqr <= 0.974624:
                                return [1.0, 0.0]
                            else:  # if CA_sqr > 0.974624
                                if CA_sqr <= 0.974949:
                                    return [0.0, 1.0]
                                else:  # if CA_sqr > 0.974949
                                    return [1.0, 0.0]
                        else:  # if data_rate > 3.720379
                            return [0.0, 1.0]
                    else:  # if CA_sqr > 0.977279
                        if rho_total <= 0.338521:
                            if data_rate <= 7.954653:
                                if rho_total <= 0.295936:
                                    return [0.6883116883116883, 0.3116883116883117]
                                else:  # if rho_total > 0.295936
                                    return [1.0, 0.0]
                            else:  # if data_rate > 7.954653
                                return [0.0, 1.0]
                        else:  # if rho_total > 0.338521
                            return [1.0, 0.0]
                else:  # if rho_total > 0.42304
                    return [1.0, 0.0]
        else:  # if data_rate > 14.509334
            return [1.0, 0.0]
    else:  # if num_flows > 10.0
        return [1.0, 0.0]





def predict_core_down(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    return "link"


def predict_core_down_weighted(data_rate, rho, rho_total, num_flows, CA_sqr, CS_sqr):
    return [0.0, 1.0]