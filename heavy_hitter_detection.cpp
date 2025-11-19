#include <iostream>
#include "CorrectDetector.hpp"
#include "Dataset.hpp"
#include "UniversalGraph.hpp"
#include <chrono>
#include "HeapUnivMon.hpp"

void Compare(std::unordered_map<std::string,WEIGHT_TYPE>& truth,std::unordered_map<std::string,WEIGHT_TYPE>& detect,ofstream& outFile){
    int correct_num = 0;
    double aae = 0;
    double are = 0;
    double precision = 0;
    double recall = 0;
    double f1 = 0;
    double sum = 0;
    for(auto elem:truth){
        sum+=elem.second;
    }
    for(auto elem:detect){
        if(truth.find(elem.first) != truth.end()){
            correct_num++;
            int res = truth[elem.first]-elem.second;
            aae += abs(res);
            if(truth[elem.first]!=0){
                are += double(abs(res))/truth[elem.first];
            }
        }
    }
    aae/=correct_num;
    are/=correct_num;
    precision = correct_num / (double)detect.size();
    recall = correct_num/(double)truth.size();
    f1 = (2*precision*recall)/(recall+precision);
    std::cout<<precision<<","<<recall<<","<<f1<<","<<are<<","<<aae<<std::endl;
    outFile<<precision<<","<<recall<<","<<f1<<","<<are<<","<<aae<<",";
}

int main(){
    g_func g = Sum;
    std::string output_path="./result/heavy_hitter.csv";
    ofstream out(output_path,std::ios::app);
    out << "algrithms" <<"," << "Memory(KB)" << "," << "precision"<<"," << "recall"<<"," << "f1" <<","<<"are"<<","<<"aae"<<","<<"throughput"<<"\n";
    std::string datasetPath="./Data/caida.dat";
    Dataset caidaDataset(datasetPath,-1);
    CorrectDetector<VERTEX_ID_TYPE,WEIGHT_TYPE> detector;
    for(int i = 0;i<caidaDataset.dataset.size();i++){
        detector.Insert(caidaDataset.dataset[i].u,caidaDataset.dataset[i].v,caidaDataset.dataset[i].weight);
    }
    uint32_t threshold = 0;
    auto res = detector.GetHeavy(vertex_top_k,lambda);
    for(auto mem : memory_list){
        std::cout<<"mem is "<<mem<<std::endl;
        mem*=1024;
        UniversalGraph<VERTEX_ID_TYPE,FP_TYPE,WEIGHT_TYPE> ug(mem,us_ratio);
        auto num = mem/HeapNode<VERTEX_ID_TYPE,WEIGHT_TYPE>::GetMemSzie();
        HeapUnivMon<VERTEX_ID_TYPE,WEIGHT_TYPE> hp(num,mem);

        out<<"UG-CAIDA"<<","<<mem/1024<<",";
        auto time_start = std::chrono::steady_clock::now();
        for(int i = 0;i<caidaDataset.dataset.size();i++){
            ug.Insert(caidaDataset.dataset[i].u,caidaDataset.dataset[i].v,caidaDataset.dataset[i].weight);
        }
        auto time_end = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_start);
        std::cout << "Throughput(Mips):     " << caidaDataset.dataset.size()/ (1000 * 1000 * time_span.count()) << std::endl<<std::endl;
        auto detect = ug.GetHeavy(vertex_top_k,lambda); 
        Compare(res,detect,out);
        out<< caidaDataset.dataset.size()/ (1000 * 1000 * time_span.count()) <<"\n";

        out<<"Strawman-CAIDA"<<","<<mem/1024<<",";
        auto time_start1 = std::chrono::steady_clock::now();
        for(int i = 0;i<caidaDataset.dataset.size();i++){
            hp.Insert(caidaDataset.dataset[i].u,caidaDataset.dataset[i].v,caidaDataset.dataset[i].weight);
        }
        auto time_end1 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span1 = std::chrono::duration_cast<std::chrono::duration<double>>(time_end1 - time_start1);
        std::cout << "Throughput(Mips):     " << caidaDataset.dataset.size()/ (1000 * 1000 * time_span1.count()) << std::endl<<std::endl;
        auto detect1 = hp.GetHeavy(lambda); 
        Compare(res,detect1,out);
        out<< caidaDataset.dataset.size()/ (1000 * 1000 * time_span1.count()) <<"\n";        
        
    }
    out.close();
    return 0;
}