/*
        The following Program is designed to inspect specific detector layers of the Full Silicon Tracking Detectors
        This includes the vertex, inner, and outer tracking detectors. 
        The inspected root file is the output of the ClusterShapeAnalysis code.
        We will focuess on the detector barrel layers, but the endcaps can also be viewed (except for the OT endcaps)
*/

#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TList.h>
#include <iostream>
#include <set>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>
#include <TPaletteAxis.h>
#include <TH3F.h>

void threeBYthree(TDirectory* dir, const std::set<std::string>& histogramNames, string homeDirec, string layer){
    // Create a main canvas
    TCanvas *c1 = new TCanvas("c1", "Cluster EDEP for 1-9 Hits in GeV", 800, 800);
    c1->Divide(3, 3); // Divide the canvas into a 3x3 grid
    int count = 0;

    std::vector<int> colors = {kRed+1, kRed, kOrange+1, kOrange, kYellow+1, kGreen+3, kGreen+4, kBlue, kViolet+2};
    TList* keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        
        if (obj->IsA()->InheritsFrom(TH1::Class())) {
            TH1* hist = (TH1*)obj;
            if (hist->GetEntries() == 0) continue; 
            // Check if the histogram name is in the set of names to plot
            if (histogramNames.find(hist->GetName()) != histogramNames.end()) {
                c1->cd(count + 1); // Move to the (i+1)th pad
                hist->SetStats(kFALSE); //disable statistics box
                // Clear the individual axis titles
                //hist->SetXTitle(""); // Clear x-axis title
                //hist->SetYTitle(""); // Clear y-axis title
                hist->SetLineColor(colors[count]);
                hist->Draw();
                gROOT->SetBatch(kTRUE); //to avoid tcanvas popping up 

                // Create a tiny legend
                TLegend *legend = new TLegend(0.5, 0.8, 0.9, 0.9); 
                legend->AddEntry(hist, Form("%i Hit Clusters", count+1), "l"); // Add entry with a label
                legend->SetTextSize(0.05); // Set text size for the legend
                legend->Draw(); // Draw the legend on the pad
                
                c1->Update();  // Update the canvas to reflect changes
                count++; 
            }
        }
    }

    //Create new directory: 
    string outputDir2 = Form("%s/%s", homeDirec.c_str(), layer.c_str());
    int dir_status = mkdir(outputDir2.c_str(), 0777);
    c1->SaveAs(Form("%s/clusterPerNumHits.png",outputDir2.c_str()));
    delete c1; 
}

void plotRatio(const std::vector<TDirectory*> dirs, const std::set<std::string>& histogramNames_hit, const std::set<std::string>& histogramNames_cluster, string homeDirec, string bORe, string normORnot, string tdr_, std::vector<string> _outThings){
    vector <int> layer; 
   
    vector <double> v1;
    vector <double> v1Error;
    vector <double> v2;
    vector <double> v2Error;
    vector <double> layers_vec;
    vector <double> ratio; //ratio for hit/cluster
    vector <double> ratioError;
    int count = 0;
    for (TDirectory* dir : dirs) {
        TList* keys = dir->GetListOfKeys();
        TIter next(keys);
        TKey* key;
        while ((key = (TKey*)next())) { 
            TObject* obj = key->ReadObj();

            if (obj->IsA()->InheritsFrom(TH1::Class())) {
                TH1* hist = (TH1*)obj;
                if (hist->GetEntries() == 0) continue; 
                // Check if the histogram name is in the set of names to plot
                if (histogramNames_hit.find(hist->GetName()) != histogramNames_hit.end()) {
                    double mean = hist->GetMean() * 3.70e-9; //put e- into GeV
                    double sme = hist->GetMeanError() * 3.70e-9; //Stadard error of the mean (std/sqrt(N))
                    v1.push_back(mean); 
                    v1Error.push_back(sme); 
                    count ++;  
                }
                // Check if the histogram name is in the set of names to plot
                if (histogramNames_cluster.find(hist->GetName()) != histogramNames_cluster.end()) {
                    double mean = hist->GetMean();
                    double sme = hist->GetMeanError(); //Stadard error of the mean (std/sqrt(N))
                    v2.push_back(mean); 
                    v2Error.push_back(sme);
                }
        }
    }
    layer.push_back(count);
}

    for(int i = 0; i < v1.size(); i++){
        ratio.push_back(v1[i]/v2[i]);
        //ratioError.push_back(1);
        ratioError.push_back((1/v2[i]) * sqrt(pow(v1Error[i], 2) + pow(v1[i]/v2[i], 2)* pow(v2Error[i],2)));
    }
    

    for(int i = 0; i < v1.size(); i++){
        layers_vec.push_back(i+1);
    }

    TGraphErrors* graph = new TGraphErrors(ratio.size(), layers_vec.data(), ratio.data(), nullptr, ratioError.data());

    // Create a canvas
    TCanvas *canvas = new TCanvas("canvas", Form("Full Silicon Tracking Detector Layers vs Hit/Cluster Energy Ratio"), 900, 600); //pixels wide v pixels high

    // Draw the graph
    gPad->SetMargin(0.1, 0.2, 0.15, 0.1);
    graph->SetTitle(Form("Full Silicon Tracking Detector Layers vs %s Hit/Cluster Energy Ratio; %s Detector Layers; Hits/Clusters Energy Ratio", tdr_.c_str() ,bORe.c_str()));
    graph->SetMarkerStyle(kFullCircle);
    graph->SetMarkerSize(0.5); 
    graph->SetMarkerColor(kBlack);
    graph->Draw("AP");

    //Create a TBox: 
    float xminl = 0.75;
    float yminl = 0.9;
    float xmaxl = 0.99;
    float ymaxl = 0.15;
    TBox *box = new TBox(xminl, yminl, xmaxl, ymaxl);  // (xmin, ymin, xmax, ymax)
    box->SetLineColor(kBlack);
    box->SetFillColor(kWhite);  // Set fill color to white
    box->SetLineWidth(2);       // Set line width for the box
    box->Draw();

    TLine *line = new TLine(graph->GetXaxis()->GetXmin(), 1, graph->GetXaxis()->GetXmax(), 1);
    line->SetLineStyle(2); // 2 corresponds to dashed line
    line->SetLineColor(14); // dark blue
    line->SetLineWidth(3);
    line->Draw();

    double yMin = graph->GetYaxis()->GetXmin();
    double yMax = graph->GetYaxis()->GetXmax();
    // Create a legend
    TLegend *legend = new TLegend(xminl, yminl, xmaxl, ymaxl); //(0.1, 0.65, 0.3, 0.9); // x1, y1, x2, y2 in normalized coordinates
    legend->AddEntry(graph, "Data Points", "p"); // Add graph to legend
    std::vector<int> colors = {kYellow, kRed, kBlue};
    std::vector<string> detector = {"VX", "IT", "OT"};
    for(int i = 0; i < layer.size(); i++){
        int xMin = 1;
        if (i != 0) xMin = layer[i-1] + 1;
        TBox *box = new TBox(xMin, yMin, layer[i], yMax);
        box->SetFillColorAlpha(colors[i], 0.3); // 30% transparency
        box->SetLineColor(colors[i]); // Outline color
        box->Draw();
        legend->AddEntry(box, Form("%s%s", detector[i].c_str(), bORe.c_str()), "f"); // Add shaded region to legend
    }
    // Add an empty entry
    legend->AddEntry((TObject*)0, "VX Sensor: 50 #mum", "");
    legend->AddEntry((TObject*)0, "IT Sensor: 100 #mum", "");
    legend->AddEntry((TObject*)0, "OT Sensor: 100 #mum", "");
    //Extra info
    if(_outThings[0] != "0") legend->AddEntry((TObject*)0, Form("#theta Range: %s", _outThings[0].c_str()), "");
    if(_outThings[1] != "0") legend->AddEntry((TObject*)0, Form("#phi Range: %s", _outThings[1].c_str()), "");
    if(_outThings[2] != "0") legend->AddEntry((TObject*)0, Form("P_{T}: %s", _outThings[2].c_str()), "");
    if(_outThings[3] != "0") legend->AddEntry((TObject*)0, Form("Particle Type: %s", _outThings[3].c_str()), "");

    legend->Draw();


    // Save the canvas as an image
    canvas->SaveAs(Form("%s/%s_ratioGraph%s.png", homeDirec.c_str(), bORe.c_str(), normORnot.c_str()));

    // Clean up
    delete canvas;
    delete graph;


}

void plotAverage(const std::vector<TDirectory*> dirs, const std::set<std::string>& histogramNames, string homeDirec, string type, string bORe, string tdr_, std::vector<string> _outThings){
    string unit;
    if (type == "time"){
        unit = "ns";
    }
    else if (type == "electrons"){
        unit = "electrons";
    }
    else if (type == "hits"){
        unit = "hits";
    }
    else{
        unit = "GeV";
    }

    vector <int> layer; 
   
    vector <double> v;
    vector <double> vError;
    vector <double> layers_vec;
    int count = 0;
    for (TDirectory* dir : dirs) {
        TList* keys = dir->GetListOfKeys();
        TIter next(keys);
        TKey* key;
        while ((key = (TKey*)next())) { 
            TObject* obj = key->ReadObj();

            if (obj->IsA()->InheritsFrom(TH1::Class())) {
                TH1* hist = (TH1*)obj;
                if (hist->GetEntries() == 0) continue; 
                // Check if the histogram name is in the set of names to plot
                if (histogramNames.find(hist->GetName()) != histogramNames.end()) {
                    double mean = hist->GetMean();
                    double sme = hist->GetMeanError(); //Stadard error of the mean (std/sqrt(N))
                    v.push_back(mean); 
                    vError.push_back(sme); 
                    count ++;  
                }
            }
        }
        layer.push_back(count);
    }

    for(int i = 0; i < v.size(); i++){
        layers_vec.push_back(i+1);
    }

    TGraphErrors* graph = new TGraphErrors(v.size(), layers_vec.data(), v.data(), nullptr, vError.data());

    // Create a canvas
    TCanvas *canvas = new TCanvas("canvas", Form("%s Full Silicon Tracking Detector Layers vs Average %s in [%s]",tdr_.c_str(),type.c_str(), unit.c_str()), 900, 600); //pixels wide v pixels high

    // Draw the graph
    gPad->SetMargin(0.1, 0.2, 0.15, 0.1);
    graph->SetTitle(Form("%s Full Silicon Tracking Detector Layers vs Average %s; %s Detector Layers; Mean %s [%s]",tdr_.c_str(), type.c_str(), bORe.c_str(), type.c_str(), unit.c_str()));
    graph->SetMarkerStyle(kFullCircle);
    graph->SetMarkerSize(0.5); 
    graph->SetMarkerColor(kBlack);
    graph->Draw("AP");

    double yMin = graph->GetYaxis()->GetXmin();
    double yMax = graph->GetYaxis()->GetXmax();
    //Create a TBox: 
    float xminl = 0.75;
    float yminl = 0.9;
    float xmaxl = 0.99;
    float ymaxl = 0.15;
    TBox *box = new TBox(xminl, yminl, xmaxl, ymaxl);  // (xmin, ymin, xmax, ymax)
    box->SetLineColor(kBlack);
    box->SetFillColor(kWhite);  // Set fill color to white
    box->SetLineWidth(2);       // Set line width for the box
    box->Draw();

    // Create a legend
    TLegend *legend = new TLegend(xminl, yminl, xmaxl, ymaxl); //(0.1, 0.65, 0.3, 0.9); // x1, y1, x2, y2 in normalized coordinates
    legend->AddEntry(graph, "Data Points", "p"); // Add graph to legend
    std::vector<int> colors = {kYellow, kRed, kBlue};
    std::vector<string> detector = {"VX", "IT", "OT"}; 
    for(int i = 0; i < layer.size(); i++){
        int xMin = 1;
        if (i != 0) xMin = layer[i-1] + 1;
        TBox *box = new TBox(xMin, yMin, layer[i], yMax);
        box->SetFillColorAlpha(colors[i], 0.3); // 30% transparency
        box->SetLineColor(colors[i]); // Outline color
        box->Draw();
        legend->AddEntry(box, Form("%s%s", detector[i].c_str(), bORe.c_str()), "f"); // Add shaded region to legend
        
    }
    // Add an empty entry
    legend->AddEntry((TObject*)0, "VX Sensor: 50 #mum", "");
    legend->AddEntry((TObject*)0, "IT Sensor: 100 #mum", "");
    legend->AddEntry((TObject*)0, "OT Sensor: 100 #mum", "");
    //Extra info
    if(_outThings[0] != "0") legend->AddEntry((TObject*)0, Form("#theta Range: %s", _outThings[0].c_str()), "");
    if(_outThings[1] != "0") legend->AddEntry((TObject*)0, Form("#phi Range: %s", _outThings[1].c_str()), "");
    if(_outThings[2] != "0") legend->AddEntry((TObject*)0, Form("P_{T}: %s", _outThings[2].c_str()), "");
    if(_outThings[3] != "0") legend->AddEntry((TObject*)0, Form("Particle Type: %s", _outThings[3].c_str()), "");

    legend->Draw();


    // Save the canvas as an image
    canvas->SaveAs(Form("%s/%s_%s_averageGraph.png", homeDirec.c_str(), bORe.c_str(), type.c_str()));

    // Clean up
    delete canvas;
    delete graph;


}

void processDirectory(TDirectory* dir, const std::set<std::string>& histogramNames, string homeDirec, string layer) {
    TList* keys = dir->GetListOfKeys();

    TIter next(keys);
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        
        if (obj->IsA()->InheritsFrom(TH1::Class())) {
            TH1* hist = (TH1*)obj;
            if (hist->GetEntries() == 0) continue; 
            // Check if the histogram name is in the set of names to plot
            if (histogramNames.find(hist->GetName()) != histogramNames.end()) {
                string fileName = hist->GetName(); 
                //Create new directory: 
                string outputDir2 = Form("%s/%s", homeDirec.c_str(), layer.c_str());
                int dir_status = mkdir(outputDir2.c_str(), 0777);
                // Create a new canvas for each histogram
                TCanvas *canvas = new TCanvas(hist->GetName(), hist->GetTitle(), 800, 600);
                if (fileName.find("_time_") != std::string::npos){
                    hist->GetXaxis()->SetRangeUser(0, 10);
                }
                if (fileName.find("hit_edep") != std::string::npos){
                    hist->GetXaxis()->SetRangeUser(0, 36000);
                }
                if (fileName.find("norm")!= std::string::npos){
                    hist->GetXaxis()->SetRangeUser(0, 0.15e-3);
                }

                hist->Draw();
                canvas->Update();  // Update the canvas to reflect changes
                gROOT->SetBatch(kTRUE); //to avoid tcanvas popping up 

                // Generate a unique file name
                canvas->SaveAs(Form("%s/%s.png",outputDir2.c_str(),fileName.c_str()));
                
                delete canvas; // Clean up the canvas
            }
        }
    }
}

void plot2DColor(TDirectory* dir, const std::set<std::string>& histogramNames, string homeDirec, string layer){
    TCanvas *canvas = new TCanvas("c1", "Time vs Energy Histogram", 800, 600);
    canvas->SetMargin(0.15, 0.15, 0.1, 0.1); // left, right, bottom, top

    // Get a 2D histogram
    TList* keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        
        if (obj->IsA()->InheritsFrom(TH1::Class())) {
            TH1* hist = (TH2*)obj;
            if (hist->GetEntries() == 0) continue; 
                // Check if the histogram name is in the set of names to plot
                if (histogramNames.find(hist->GetName()) != histogramNames.end()) {
                    string fileName = hist->GetName(); 
                    hist->SetStats(kFALSE); // Disable statistics box
                    //Create new directory: 
                    string outputDir2 = Form("%s/%s", homeDirec.c_str(), layer.c_str());
                    int dir_status = mkdir(outputDir2.c_str(), 0777);
                    hist->Draw("COLZ");
                    // Add a color bar
                    TPaletteAxis *palette = new TPaletteAxis(0.85, 0.1, 0.9, 0.9, hist->GetMinimum(), hist->GetMaximum());
                    palette->SetTitle("Number of Hits");
                    palette->SetLabelSize(0.03);
                    palette->Draw("SAME");
        
                    canvas->Update();  // Update the canvas to reflect changes
                    gROOT->SetBatch(kTRUE); //to avoid tcanvas popping up 

                    // Generate a unique file name
                    canvas->SaveAs(Form("%s/%s.png",outputDir2.c_str(),fileName.c_str()));
                    
                    delete canvas; // Clean up the canvas
                }
            }
        }
}

    void processDirectory3D(const std::vector<TDirectory*>& dirs, const std::set<std::string>& histogramNames, string homeDirec) {
    // Create a ROOT file to save the histogram
    TFile *outputFile = new TFile(Form("%s/histograms.root", homeDirec.c_str()), "RECREATE");

    // Create a new canvas for each histogram
    TCanvas *canvas = new TCanvas("Histogram 3D","3D Barrel Histograms", 1500, 1300);
    //Create TLegend: 
    TLegend *legend = new TLegend(0.5, 0.8, 0.6, 0.9); 
    std::vector<int> colors = {kRed+1,kBlue, kGreen+3, kBlue, kOrange+1, kViolet+2};
    std::vector<string> detector = {"VXB", "ITB", "OTB"};
    string outputDir2;
    int counter = 0;
    string fileName;
    // Get the mean of each axis (X, Y, Z)
    double meanX=-1;
    double meanY=-1;
    double meanZ=-1;

    // Get the standard deviation of each axis (X, Y, Z)
    double stdDevX = -1;  // Std Dev of X axis
    double stdDevY = -1;  // Std Dev of Y axis
    double stdDevZ = -1;  // Std Dev of Z axis
    
    for (TDirectory* dir : dirs) {
        TList* keys = dir->GetListOfKeys();
        TIter next(keys);
        TKey* key;
        while ((key = (TKey*)next())) {
            TObject* obj = key->ReadObj();
            if (obj->IsA()->InheritsFrom(TH1::Class())) {
                TH1* hist = (TH1*)obj;
                if (hist->GetEntries() == 0) continue; 
                // Check if the histogram name is in the set of names to plot
                if (histogramNames.find(hist->GetName()) != histogramNames.end()) {
                    fileName = hist->GetName(); 
                    //Create new directory: 
                    outputDir2 = Form("%s", homeDirec.c_str());
                    int dir_status = mkdir(outputDir2.c_str(), 0777);
                    hist->SetLineColor(colors[counter]);

                    hist->GetXaxis()->SetTickLength(0.005);
                    hist->GetYaxis()->SetTickLength(0.005);
                    hist->GetZaxis()->SetTickLength(0.005);
                   
                    if(counter == 0){
                        hist->Draw("");
                        // Add a color bar
                        TPaletteAxis *palette = new TPaletteAxis(0.3, 0.6, 0.34, 0.95, hist->GetMinimum(), hist->GetMaximum());
                        //palette->SetTitle("Number of Hits");
                        palette->SetLabelSize(0.03);
                        palette->SetNdivisions(5);
                        palette->Draw("SAME");
                    }
                    else{
                        hist->Draw("SAME");
                    }

                
                    if(counter == 2){
                        meanX = hist->GetMean(1);  // Mean of X axis
                        stdDevX = hist->GetStdDev(1);  // Std Dev of X axis
                        meanY = hist->GetMean(2);  // Mean of X axis
                        stdDevY = hist->GetStdDev(2);
                        meanZ = hist->GetMean(3);  // Mean of X axis
                        stdDevZ = hist->GetStdDev(3); 
                    }
                    
                    //gROOT->SetBatch(kTRUE); //to avoid tcanvas popping up 
                    double factor = 5;
                    //Set the X, Y, and Z axis ranges manually
                    if(stdDevX == 0||stdDevY == 0||stdDevZ == 0){
                        //Set the X, Y, and Z axis ranges manually
                        //hist->GetXaxis()->SetRangeUser(0, 1000);
                        //hist->GetYaxis()->SetRangeUser(0, 1000);
                        hist->GetZaxis()->SetRangeUser(-2500, 800);
                    }
                    else{
                        //hist->GetXaxis()->SetRangeUser(meanX - factor*stdDevX, meanX + factor*stdDevX);
                        //hist->GetYaxis()->SetRangeUser(meanY - factor*stdDevY, meanY + factor*stdDevY);
                        hist->GetZaxis()->SetRangeUser(meanZ - factor*stdDevZ, meanZ + factor*stdDevZ);
                    }
                    legend->AddEntry(hist, Form("%s", detector[counter].c_str()), "f");
                    canvas->Update();  // Update the canvas to reflect changes
                    }
                }
            }
            
            counter++;
        }

    legend->Draw();
    canvas->SaveAs(Form("%s/%s.png",outputDir2.c_str(), fileName.c_str()));
    //canvas->SaveAs(Form("%s/fileName.png.svg",outputDir2.c_str()));

    // Enable interactive rotation with the mouse

    canvas->SetTheta(30);  // Initial rotation angle
    canvas->SetPhi(20);   // Initial rotation angle
    canvas->Update(); 
    // Save the histogram to the ROOT file
    canvas->Write();
    legend->Write();
    // Save the entire canvas and histograms into the ROOT file
    outputFile->Close();  // This saves and closes the file

    delete canvas; // Clean up the canvas

}

void layer_analysis(const char *inputFile, std::string s0 = "0", std::string s1 = "0", std::string s2 = "0", std::string s3 = "0"){
    string tdr = "Digitized"; 
    std::vector<string> outThings = {s0, s1, s2, s3};
    TFile* file = TFile::Open(inputFile);
    
    TDirectory* dirMain = (TDirectory*)file->Get("MyClusterShapeAnalysis");
    TDirectory* dir_vb = (TDirectory*)dirMain->Get("clusters_vb");
    TDirectory* dir_ve = (TDirectory*)dirMain->Get("clusters_ve");
    TDirectory* dir_ib = (TDirectory*)dirMain->Get("clusters_ib");
    TDirectory* dir_ie = (TDirectory*)dirMain->Get("clusters_ie");
    TDirectory* dir_ob = (TDirectory*)dirMain->Get("clusters_ob");
    TDirectory* dir_oe = (TDirectory*)dirMain->Get("clusters_oe");// NOT YET

    //create output directory: 
    string outputDir = "layer_plots";
    int dir_status = mkdir(outputDir.c_str(), 0777);

    // Set of histogram names to plot
    std::set<std::string> histogramsToPlot_time = {"trackerhit_time_layer0", "trackerhit_time_layer1", "trackerhit_time_layer2", "trackerhit_time_layer3", "trackerhit_time_layer4", "trackerhit_time_layer5", "trackerhit_time_layer6", "trackerhit_time_layer7", "trackerhit_time_layer8"};
    std::set<std::string> histogramsToPlot_truthEdep = {"h_truth_cluster_edep_layer0", "h_truth_cluster_edep_layer1", "h_truth_cluster_edep_layer2", "h_truth_cluster_edep_layer3", "h_truth_cluster_edep_layer4", "h_truth_cluster_edep_layer5", "h_truth_cluster_edep_layer6", "h_truth_cluster_edep_layer7", "h_truth_cluster_edep_layer8"};
    std::set<std::string> histogramsToPlot_clusterEdep = {"Clusters_edep_layer0", "Clusters_edep_layer1", "Clusters_edep_layer2", "Clusters_edep_layer3", "Clusters_edep_layer4", "Clusters_edep_layer5", "Clusters_edep_layer6", "Clusters_edep_layer7", "Clusters_edep_layer8"};
    std::set<std::string> histogramsToPlot_hitEdep = {"hit_edep_layer0", "hit_edep_layer1", "hit_edep_layer2", "hit_edep_layer3", "hit_edep_layer4", "hit_edep_layer5", "hit_edep_layer6", "hit_edep_layer7", "hit_edep_layer8"};
    std::set<std::string> histogramsToPlot_clusterEdep_norm = {"Clusters_edep_norm_layer0", "Clusters_edep_norm_layer1", "Clusters_edep_norm_layer2", "Clusters_edep_norm_layer3", "Clusters_edep_norm_layer4", "Clusters_edep_norm_layer5", "Clusters_edep_norm_layer6", "Clusters_edep_norm_layer7", "Clusters_edep_norm_layer8"};
    std::set<std::string> histogramsToPlot_clusterEdep_byHitDensity = {"cluster_1hits","cluster_2hits","cluster_3hits","cluster_4hits","cluster_5hits","cluster_6hits","cluster_7hits","cluster_8hits","cluster_9hits"};
    //hit cluster edep diff plots: 
    std::set<std::string> histogramsToPlot_diffEDEP = {"diffHitCluster_edep_layer0", "diffHitCluster_edep_layer1", "diffHitCluster_edep_layer2", "diffHitCluster_edep_layer3", "diffHitCluster_edep_layer4", "diffHitCluster_edep_layer5", "diffHitCluster_edep_layer6", "diffHitCluster_edep_layer7", "diffHitCluster_edep_layer8"};
    std::set<std::string> histrogramsNumHitsPerLayer = {"thclen_layer0", "thclen_layer1", "thclen_layer2", "thclen_layer3", "thclen_layer4", "thclen_layer5", "thclen_layer6", "thclen_layer7", "thclen_layer8"};
    std::set<std::string> histogramsCEDEPvHitNum = {"edepVhits_layer0","edepVhits_layer1","edepVhits_layer2","edepVhits_layer3","edepVhits_layer4","edepVhits_layer5","edepVhits_layer6","edepVhits_layer7"};
    //theta, r, z greater than 20 cluster hits
    std::set<std::string> histograms_theta = {"theta_20hit", "theta_20hit_layer0", "theta_20hit_layer1", "theta_20hit_layer2", "theta_20hit_layer3", "theta_20hit_layer4", "theta_20hit_layer5", "theta_20hit_layer6", "theta_20hit_layer7"};
    std::set<std::string> histograms_r = {"r_20hit", "r_20hit_layer0", "r_20hit_layer1", "r_20hit_layer2", "r_20hit_layer3", "r_20hit_layer4", "r_20hit_layer5", "r_20hit_layer6", "r_20hit_layer7"};
    std::set<std::string> histograms_z = {"z_20hit", "z_20hit_layer0", "z_20hit_layer1", "z_20hit_layer2", "z_20hit_layer3", "z_20hit_layer4", "z_20hit_layer5", "z_20hit_layer6", "z_20hit_layer7"};

    //create array to plot
    std::vector<TDirectory*> directories_b = {dir_vb, dir_ib, dir_ob};
    std::vector<TDirectory*> directories_e = {dir_ve, dir_ie, dir_oe};

    //ALL OTE REAL PLOTS: 
    processDirectory(dir_oe, histogramsToPlot_time, outputDir, "clusters_oe");
    processDirectory(dir_oe, histogramsToPlot_clusterEdep, outputDir, "clusters_oe");
    processDirectory(dir_oe, histogramsToPlot_hitEdep, outputDir, "clusters_oe");

    
    //process each directory
    processDirectory(dir_vb, histogramsToPlot_time, outputDir, "clusters_vb");
    processDirectory(dir_vb, histogramsToPlot_clusterEdep, outputDir, "clusters_vb");
    processDirectory(dir_vb, histogramsToPlot_hitEdep, outputDir, "clusters_vb");
    processDirectory(dir_ib, histogramsToPlot_time, outputDir, "clusters_ib");
    processDirectory(dir_ib, histogramsToPlot_clusterEdep, outputDir, "clusters_ib");
    processDirectory(dir_ib, histogramsToPlot_hitEdep, outputDir, "clusters_ib");
    processDirectory(dir_ob, histogramsToPlot_time, outputDir, "clusters_ob");
    processDirectory(dir_ob, histogramsToPlot_clusterEdep, outputDir, "clusters_ob");
    processDirectory(dir_ob, histogramsToPlot_hitEdep, outputDir, "clusters_ob");

    //diff plots for b: 
    // processDirectory(dir_vb, histogramsToPlot_diffEDEP, outputDir, "clusters_vb");
    // processDirectory(dir_ib, histogramsToPlot_diffEDEP, outputDir, "clusters_ib");
    // processDirectory(dir_ob, histogramsToPlot_diffEDEP, outputDir, "clusters_ob");


    processDirectory(dir_vb, histogramsToPlot_truthEdep, outputDir, "clusters_vb");
    //EndCap: 
    processDirectory(dir_ve, histogramsToPlot_time, outputDir, "clusters_ve");
    processDirectory(dir_ve, histogramsToPlot_clusterEdep, outputDir, "clusters_ve");
    processDirectory(dir_ve, histogramsToPlot_hitEdep, outputDir, "clusters_ve");
    processDirectory(dir_ie, histogramsToPlot_time, outputDir, "clusters_ie");
    processDirectory(dir_ie, histogramsToPlot_clusterEdep, outputDir, "clusters_ie");
    processDirectory(dir_ie, histogramsToPlot_hitEdep, outputDir, "clusters_ie");
    //Normalize Cluster EDEP plots:
    // processDirectory(dir_vb, histogramsToPlot_clusterEdep_norm, outputDir, "norm_clusters_vb");
    // processDirectory(dir_ve, histogramsToPlot_clusterEdep_norm, outputDir, "norm_clusters_ve");
    // processDirectory(dir_ib, histogramsToPlot_clusterEdep_norm, outputDir, "norm_clusters_ib");
    // processDirectory(dir_ie, histogramsToPlot_clusterEdep_norm, outputDir, "norm_clusters_ie"); 
    // processDirectory(dir_ob, histogramsToPlot_clusterEdep_norm, outputDir, "norm_clusters_ob");

    //average plots
    plotAverage(directories_b, histogramsToPlot_time, outputDir, "time", "B", tdr, outThings);//B for barrel 
    plotAverage(directories_b, histogramsToPlot_clusterEdep, outputDir, "EDEP", "B", tdr, outThings);
    plotAverage(directories_b, histogramsToPlot_hitEdep, outputDir, "electrons", "B", tdr, outThings);
    plotAverage(directories_e, histogramsToPlot_time, outputDir, "time", "E", tdr, outThings);//B for barrel 
    plotAverage(directories_e, histogramsToPlot_clusterEdep, outputDir, "EDEP", "E", tdr, outThings);
    plotAverage(directories_e, histogramsToPlot_hitEdep, outputDir, "electrons", "E", tdr, outThings);
    //TRUTH: 
    // plotAverage(directories_b, histogramsToPlot_truthEdep, outputDir, "EDEPT", "B", "Truth");
    // plotAverage(directories_e, histogramsToPlot_truthEdep, outputDir, "EDEPT", "E", "Truth");
    
    //average hits plot
    plotAverage(directories_b, {"thclen_layer0", "thclen_layer1", "thclen_layer2", "thclen_layer3", "thclen_layer4", "thclen_layer5", "thclen_layer6", "thclen_layer7", "thclen_layer8"}, outputDir, "hits", "B", tdr, outThings);
    plotAverage(directories_e, {"thclen_layer0", "thclen_layer1", "thclen_layer2", "thclen_layer3", "thclen_layer4", "thclen_layer5", "thclen_layer6", "thclen_layer7", "thclen_layer8"}, outputDir, "hits", "E", tdr, outThings);//B for barrel 

    plotAverage(directories_b, {"theta_20hit_layer0", "theta_20hit_layer1", "theta_20hit_layer2", "theta_20hit_layer3", "theta_20hit_layer4", "theta_20hit_layer5", "theta_20hit_layer6", "theta_20hit_layer7"}, outputDir, "theta", "B", tdr, outThings);
    plotAverage(directories_b, {"r_20hit_layer0", "r_20hit_layer1", "r_20hit_layer2", "r_20hit_layer3", "r_20hit_layer4", "r_20hit_layer5", "r_20hit_layer6", "r_20hit_layer7"}, outputDir, "r", "B", tdr, outThings);
    plotAverage(directories_b,{"z_20hit_layer0", "z_20hit_layer1", "z_20hit_layer2", "z_20hit_layer3", "z_20hit_layer4", "z_20hit_layer5", "z_20hit_layer6", "z_20hit_layer7"} , outputDir, "z", "B", tdr, outThings);
    
    //Process Directory for Each Hit per Layer + add the Hits in general: 
    processDirectory(dir_vb, histrogramsNumHitsPerLayer, outputDir, "clusters_vb");
    processDirectory(dir_ve, histrogramsNumHitsPerLayer, outputDir, "clusters_ve");
    processDirectory(dir_ob, histrogramsNumHitsPerLayer, outputDir, "clusters_ob");
    processDirectory(dir_oe, histrogramsNumHitsPerLayer, outputDir, "clusters_oe");
    processDirectory(dir_ib, histrogramsNumHitsPerLayer, outputDir, "clusters_ib");
    processDirectory(dir_ie, histrogramsNumHitsPerLayer, outputDir, "clusters_ie");

    processDirectory(dir_vb, {"thclen"}, outputDir, "clusters_vb");
    processDirectory(dir_ve, {"thclen"}, outputDir, "clusters_ve");
    processDirectory(dir_ob, {"thclen"}, outputDir, "clusters_ob");
    processDirectory(dir_oe, {"thclen"}, outputDir, "clusters_oe");
    processDirectory(dir_ib, {"thclen"}, outputDir, "clusters_ib");
    processDirectory(dir_ie, {"thclen"}, outputDir, "clusters_ie");



    // --- RATIO EASTER PLOTS -- //
    plotRatio(directories_b, histogramsToPlot_hitEdep, histogramsToPlot_clusterEdep, outputDir, "B", "", tdr, outThings);
    plotRatio(directories_e, histogramsToPlot_hitEdep, histogramsToPlot_clusterEdep, outputDir, "E", "", tdr, outThings);
    //normalized ratio: 
    plotRatio(directories_b, histogramsToPlot_hitEdep, histogramsToPlot_clusterEdep_norm, outputDir, "B", "_norm", tdr, outThings);
    plotRatio(directories_e, histogramsToPlot_hitEdep, histogramsToPlot_clusterEdep_norm, outputDir, "E", "_norm", tdr, outThings);

    //THREE BY THREE
    threeBYthree(dir_vb, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_vb");
    threeBYthree(dir_ve, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_ve");
    threeBYthree(dir_ib, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_ib");
    threeBYthree(dir_ie, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_ie");
    threeBYthree(dir_ob, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_ob");
    threeBYthree(dir_ob, histogramsToPlot_clusterEdep_byHitDensity, outputDir, "clusters_oe");

    //color bar plot: 
    plot2DColor(dir_vb, {"toa_vs_edepCluster"}, outputDir, "clusters_vb");
    plot2DColor(dir_ve, {"toa_vs_edepCluster"}, outputDir, "clusters_ve");
    plot2DColor(dir_ib, {"toa_vs_edepCluster"}, outputDir, "clusters_ib");
    plot2DColor(dir_ie, {"toa_vs_edepCluster"}, outputDir, "clusters_ie");
    plot2DColor(dir_ob, {"toa_vs_edepCluster"}, outputDir, "clusters_ob");
    plot2DColor(dir_oe, {"toa_vs_edepCluster"}, outputDir, "clusters_oe");
    
    //2D plot of cluster edep vs hit number: 
    plot2DColor(dir_vb, {"edepVhits"}, outputDir, "clusters_vb");
    plot2DColor(dir_ve, {"edepVhits"}, outputDir, "clusters_ve");
    plot2DColor(dir_ib, {"edepVhits"}, outputDir, "clusters_ib");
    plot2DColor(dir_ie, {"edepVhits"}, outputDir, "clusters_ie");
    plot2DColor(dir_ob, {"edepVhits"}, outputDir, "clusters_ob");
    plot2DColor(dir_oe, {"edepVhits"}, outputDir, "clusters_oe");

    //2D plot of cluster edep vs hit number: 
    plot2DColor(dir_vb, {"2D_r_hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_r_hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_r_hitNum"}, outputDir, "clusters_ob");
    plot2DColor(dir_vb, {"2D_r_20hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_r_20hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_r_20hitNum"}, outputDir, "clusters_ob");
    
    plot2DColor(dir_vb, {"2D_z_hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_z_hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_z_hitNum"}, outputDir, "clusters_ob");
    plot2DColor(dir_vb, {"2D_z_20hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_z_20hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_z_20hitNum"}, outputDir, "clusters_ob");

    plot2DColor(dir_vb, {"2D_theta_hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_theta_hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_theta_hitNum"}, outputDir, "clusters_ob");
    plot2DColor(dir_vb, {"2D_theta_20hitNum"}, outputDir, "clusters_vb");
    plot2DColor(dir_ib, {"2D_theta_20hitNum"}, outputDir, "clusters_ib");
    plot2DColor(dir_ob, {"2D_theta_20hitNum"}, outputDir, "clusters_ob");

    //theta r z: 
    //process each directory
    processDirectory(dir_vb, histograms_theta, outputDir, "clusters_vb");
    processDirectory(dir_vb, histograms_r, outputDir, "clusters_vb");
    processDirectory(dir_vb, histograms_z, outputDir, "clusters_vb");
    processDirectory(dir_ib, histograms_theta, outputDir, "clusters_ib");
    processDirectory(dir_ib, histograms_r, outputDir, "clusters_ib");
    processDirectory(dir_ib, histograms_z, outputDir, "clusters_ib");
    processDirectory(dir_ob, histograms_theta, outputDir, "clusters_ob");
    processDirectory(dir_ob, histograms_r, outputDir, "clusters_ob");
    processDirectory(dir_ob, histograms_z, outputDir, "clusters_ob");



    //and per layer: 
    plot2DColor(dir_vb, histogramsCEDEPvHitNum, outputDir, "clusters_vb");
    plot2DColor(dir_ve, histogramsCEDEPvHitNum, outputDir, "clusters_ve");
    plot2DColor(dir_ib, histogramsCEDEPvHitNum, outputDir, "clusters_ib");
    plot2DColor(dir_ie, histogramsCEDEPvHitNum, outputDir, "clusters_ie");
    plot2DColor(dir_ob, histogramsCEDEPvHitNum, outputDir, "clusters_ob");
    plot2DColor(dir_oe, histogramsCEDEPvHitNum, outputDir, "clusters_oe");

    //3D Histogram stuff!! 
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_digi"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_cdigi"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_20digi"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_20cdigi"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_r_z_hit"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_r_z_20hit"}, outputDir);
    
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_theta_r_hit"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_theta_r_20hit"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_theta_z_hit"}, outputDir);
    processDirectory3D({dir_vb, dir_ib, dir_ob}, {"3DPosition_theta_z_20hit"}, outputDir);
    


    //3 hit cluster stuff: 
    // processDirectory(dir_vb, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_vb");
    // processDirectory(dir_ve, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_ve");
    // processDirectory(dir_ib, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_ib");
    // processDirectory(dir_ie, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_ie"); 
    // processDirectory(dir_ob, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_ob");
    // processDirectory(dir_oe, {"3hitEDEP_vs_clusterEDEP1", "3hitEDEP_vs_clusterEDEP2", "3hitEDEP_vs_clusterEDEP3"}, outputDir, "clusters_oe");

    //Close File: 
    file->Close();


}
