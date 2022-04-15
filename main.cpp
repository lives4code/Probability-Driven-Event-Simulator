#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <queue>
#include <vector>
#include <deque>
#include <math.h>
#include <queue>
#include <map>
//#include <boost/math/distributions/pareto.hpp>
using namespace std;

struct file {
    float size;
    int idNum;
    float popularity;

    void printSize() {
        cout << "fileSize:" << size << endl;
    }
};
struct packetRequest {
    float timeBegin;
    float timeEnd;
    float FIFOtimeBegin;
    float FIFOtimeEnd;
    file* requestedFile;
    void printRequest(){
        cout << "timeBegin:" << timeBegin << endl;
        cout << "timeEnd:" << timeEnd << endl;
        cout << "FIFOBegin" << FIFOtimeBegin << endl;
        cout << "FIFOEnd" << FIFOtimeEnd << endl;
        cout << "requestedFile:" << requestedFile->idNum << endl;
        cout << "time delta:" << getTimeDelta() << endl;
        cout << "FIFODelta:" << getFIFODelta() << endl;
        cout << endl;
    }
    float getFIFODelta(){
        return FIFOtimeEnd - FIFOtimeBegin;
    }
    float getTimeDelta(){
        return timeEnd - timeBegin;
    }
};
struct packetQueue{
    deque<file*> q;
    int option;
    float maxSize;
    float currentSize;
    void initialize(int max, int _option){
        currentSize = 0; 
        maxSize = max;
        option = _option;
    }
    
    void deleteFileFromQ(){
        switch (option)
        {
        case 1:
            currentSize -= q.front()->size;
            q.pop_front();
            break;
        case 2:
        {
            float largestFile = 0;
            auto eraseable = q.begin();
            for(auto it = q.begin(); it != q.end(); it++){
                if((*it)->size > largestFile){
                    largestFile = (*it)->size;
                    eraseable = it;
                }
            }
            largestFile = 0;
            currentSize -= (*eraseable)->size;
            q.erase(eraseable);
        }
            break;
        
        default:
            q.pop_back();
            break;
        }
    }

    void insertFile(file* f){
        switch(option){
            case 1:
                if(currentSize + f->size < maxSize){
                    q.push_back(f);
                    currentSize += f->size;
                }
                else{
                    deleteFileFromQ();
                    insertFile(f);
                }
            break;

            case 2: 
            if(currentSize + f->size < maxSize){
                q.push_back(f);
                currentSize += f->size;
            }
            else{
                deleteFileFromQ();
                insertFile(f);
            }
            break;
        }

    }
    bool searchFile(file* f){
        for(auto it = q.begin(); it != q.end(); it++){
            if(f->idNum == (*it)->idNum){
                return true;
            }
        }
        return false;
    }
};
struct FIFOQueue{
    float requestAccessBandwidth = 1.5;
    queue<packetRequest*> q;
    float time;
    void initialize(){
        time = 0;
    }
    void insertPacket(packetRequest* p){
        if(time < p->timeBegin){
            time = p->timeBegin;
        }
        p->FIFOtimeBegin = time;
        q.push(p);        
    }
    void deletePacket(){
        packetRequest* current = q.front();
        //cout << "time is:" << time << endl;
        time += current->requestedFile->size/requestAccessBandwidth;
        //cout << "now time is:" << time << endl;
        current->FIFOtimeEnd = time;
        //cout << "FIFO time is " << current->getFIFODelta() << endl;
        q.pop();
    }
};


//=================================================================================================================//
//I htink this is what allows me to put the randomizer in funcitons and it give me different results.
std::default_random_engine & global_urng(){
    static std::default_random_engine u{};
    return u;
}
int pick_a_number( int from, int thru );
void randomize();
void getInput(int* optionp, int* numFiles, int* fileSizeLambda, float* paretoFileSizeAlpha, float* paretoFileSizeBeta, int* maxP, int* numSeconds, float* frpa, float* frpb, int argc, char* argv[]);
void MakeRequest(packetRequest* p, packetQueue& q, FIFOQueue& pq);
float getCacheTime(float& filesize, float& Rc);
float getInstitutionTime(float& filesize, float& Ra);
void getFIFOTime(packetRequest* p, FIFOQueue& q);
int pickPoisson(int requestLambda);
float getUniform(float min, float max);
float generatePareto(float shapeA, float scaleB);
float generateParetoSample(float shapeA, float scaleB);
int   generateFileNum(int numFiles, int fileParetoA, int fileParetoBeta);
int generateFileNum(map <int, int>& m);
map <int, int>  generateFileNumMap(int numFiles, int fileParetoA, int fileParetoBeta);




int main(int argc, char* argv[]) {
    
    //initialize variables
    struct file file1;
    struct packetRequest p;
    string input;
    file* f;
    int numFiles = 0;
    int fileSizeLambda = 0;
    float fileSizeParetoAlpha = 0;
    float fileSizeParetoBeta = 0;
    float fileRequestParetoAlpa = 0;
    float fileRequestParetoBeta = 0; 
    int max = 0;
    int option= 0;
    int nSeconds = 0;
    int* numFilesP = &numFiles;
    int* fileSizeP = &fileSizeLambda;
    float* fpas = &fileSizeParetoAlpha;
    float* fpbs = &fileSizeParetoBeta;
    float* frpa = &fileRequestParetoAlpa;
    float* frpb = &fileRequestParetoBeta;
    int* maxP = &max;
    int* numSeconds = &nSeconds;
    int* optionp = &option;
    packetQueue q;
    FIFOQueue pq;

    //either get file input or user input
    getInput(optionp, numFilesP, fileSizeP, fpas, fpbs, maxP, numSeconds, frpa, frpb, argc, argv);

    //initialize files, cache, FIFOcache, and randomizer.
    file* fileArray = new file[numFiles];
    randomize();
    q.initialize(max, option);
    pq.initialize();
    map<int, int> m = generateFileNumMap(numFiles, fileRequestParetoAlpa, fileRequestParetoBeta);

    for (int i = 0; i < numFiles; ++i) {
        fileArray[i].size = generatePareto(fileSizeParetoAlpha, fileSizeParetoBeta);
        fileArray[i].idNum = i;
    }

  
    f = &fileArray[1];
    p.requestedFile = &(fileArray[1]);
    p.timeBegin = 0;
    packetRequest* p1 = &p;
    MakeRequest(p1, q, pq);
           
    vector <float> timeTaken;
    //simulates i seconds with j file requests per second. 
    for(int i = 0; i < nSeconds; i++){
        for(int j = 0; j < pickPoisson(fileSizeLambda); j++){
            p1->timeBegin = i;
            p1->requestedFile = &(fileArray[generateFileNum(m)]);
            MakeRequest(p1, q, pq);
            timeTaken.push_back(p1->getTimeDelta());
        }
    }
    ofstream outfile;
    outfile.open("output.csv"); 
    for(int i = 0; i < timeTaken.size(); i++){
        outfile << i;
        outfile << ",";
        outfile << timeTaken.at(i);
        outfile << "\n";    
    }
    outfile.close();   
    cout << "done" << endl;
    delete fileArray;
    return 0;
}

//========================================================================================================//

int generateFileNum(map <int, int>& m){
    return m[pick_a_number(0, m.size())];
}

map <int, int>  generateFileNumMap(int numFiles, int fileParetoA, int fileParetoBeta){
    map <int, int> paretoMap;
    vector <float> paretoStore;

    //step one generate paretos for the files keep a sum of the total. 
    float sum = 0; 
    for(int i = 0; i < numFiles; i++){
        float x = generateParetoSample(fileParetoA, fileParetoBeta);
        paretoStore.push_back(x);
        //cout << x << endl;
        sum += x;
    }
    // step two divide each file by the sum total and then scale up so it can be rounded to a whole number.
    float sum2 = 0;
    for(int i = 0; i < numFiles; i++){
        paretoStore.at(i) = ((paretoStore.at(i) * 10000.0) / sum);
        sum2 += paretoStore.at(i);
    }
    // step three fill the map with the file number i the number of times in the vector generated earlier.
    int position = 0;
    for(int i = 0; i < numFiles; i++){
        for(int j =0; j < paretoStore.at(i); j++){
            paretoMap[position] = i;
            position++;
        }
    }
    //draw a random number from the distribution. 
    return paretoMap;

}


void randomize(){
static std::random_device rd{};
global_urng().seed( rd() );
}

float generateParetoSample(float shapeA, float scaleB){
    // t = xm/U^(1/a) -xm scale, a-shape
    return scaleB/(pow(getUniform(0, 1), (1/shapeA)));
}
float generatePareto(float shapeA, float scaleB){
    return scaleB/(pow((1- getUniform(0, 1)), (1/shapeA)));
}
float getUniform(float min, float max){
    static std::uniform_real_distribution<> d{};
    using parm_t = decltype(d)::param_type;
    //return d( global_urng(), parm_t(min, max)));
    return d( global_urng(), parm_t(min, max));

}
int pickPoisson(int requestLambda){
    static std::poisson_distribution<> d{};
    using parm_t = decltype(d)::param_type;
    return d( global_urng(), parm_t(requestLambda));
}
int pick_a_number( int from, int thru ){
    static std::uniform_int_distribution<> d{};
    using parm_t = decltype(d)::param_type;
    return d( global_urng(), parm_t{from, thru} );
}

void MakeRequest(packetRequest* p, packetQueue& q, FIFOQueue& pq){
    //si = file size in Mb
    //D = 400ms
    float timeFromserverToNewtork = .4;
    //RA 15Mb/s to download files
    float requestAccessBandwidth = 1.5;
    //Rc = 100 or 1000 Mb/s
    float networkBandwidth = 100;
    
    //if in cache time is Si/Rc
    if(q.searchFile(p->requestedFile)){
        p->timeEnd = p->timeBegin + getCacheTime(p->requestedFile->size, networkBandwidth);
        //cout << "in q" << endl;
    }
    //if not in cache time is D + TimeAtQueue + TransmissionTime at access Link + Transmission time from cache
    else{
        //cout << "not in queue" << endl;
        q.insertFile(p->requestedFile);
        //cout << "inserted" << endl;
        getFIFOTime(p, pq);
        p->timeEnd = p->timeBegin + getCacheTime(p->requestedFile->size, networkBandwidth) + timeFromserverToNewtork + p->getFIFODelta();
        //p->printRequest();
    }
}
float getCacheTime(float& filesize, float& Rc){
    //transmission time from cache = Si/Rc 

    return (float)filesize/Rc;
}
float getInstitutionTime(float& filesize, float& Ra){
    //transmission time at access link = Si/Ra
    return (float)filesize/Ra;
}
void getFIFOTime(packetRequest* p, FIFOQueue& q){
    q.insertPacket(p);
    q.deletePacket();
    //TODO figure out how to know how much time the packet was in FIFOqueue.
}
void getInput(int* optionp, int* numFiles, int* fileSizeLambda, float* paretoFileSizeAlpha, float* paretoFileSizeBeta, int* maxP, int* numSeconds, float* frpa, float* frpb,  int argc, char* argv[]) {
    string input;
    if (argc == 1) {
        cout << "which option would you like for queue deletion 1:largest, 2: oldest";
        cin >> input;
        *optionp = stoi(input);
        cout << "how many files would you like to generate?" << endl;
        cin >> input;
        *numFiles = stoi(input);

        cout << "what poisson variable would you like for the size of files?" << endl;
        cin >> input;
        *fileSizeLambda = stoi(input);

        cout << "what would you like the file size alpha to be?" << endl;
        cin >> input;
        *paretoFileSizeAlpha = stof(input);

        cout << "what would you like the file size beta to be?" << endl;
        cin >> input;
        *paretoFileSizeBeta = stof(input);

        cout << "how large would you like the cache?" << endl;
        cin >> input;
        *maxP = stoi(input);

        cout << "how many seconds would you like to run the program?" << endl;
        cin >> input;
        *numSeconds = stoi(input);

        cout << "what would you like the file request alpha paramater to be?" << endl;
        cin >> input; 
        *frpa = stof(input);

        cout << "what would you like the file request beta paramater to be?" << endl;
        cin >> input;
        *frpb = stof(input);
    }
    else if (argc == 2) {
        ifstream inFile;
        inFile.open(argv[1]);
        if (inFile.is_open()) {
            getline(inFile, input);
            getline(inFile, input);
            cout << "cache policy option 1-oldest 2-largest: " << input << endl;
            *optionp = stoi(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "Number of files:" << input << endl;
            *numFiles = stoi(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "File Size Poisson value:" << input << endl;
            *fileSizeLambda = stoi(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "pareto file alpha:" << input << endl;
            *paretoFileSizeAlpha = stof(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "pareto file beta:" << input << endl;
            *paretoFileSizeBeta = stof(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "queueSize:" << input << endl;
            *maxP = stoi(input);
            
            getline(inFile, input);
            getline(inFile, input);
            cout << "numSeconds:" << input << endl;
            *numSeconds = stoi(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "file request pareto alpha:" << input << endl;
            *frpa = stof(input);

            getline(inFile, input);
            getline(inFile, input);
            cout << "file request pareto beta:" << input << endl;
            *frpb = stof(input);


            inFile.close();
        }
        else {
            cout << "failed to open file" << endl;
        }
    }
    else {
        cout << "error: too many arguments" << endl;
    }

}