#include<algorithm>
#include <stdio.h>
#include <chrono>
#include <omp.h>
#include <unistd.h>
#include<list>
#include <cstdlib>
#include <bits/stdc++.h>
#include<fstream>
#define int long long int
#define rep(i,a,b) for(int i=a;i<b;i++)
#define ford(i,a,b) for(int i=a;i>=b;i--)
#define fori(it,A) for(it=A.begin();it!=A.end();it++)
#define out1(a) cout<<#a<<" "<<a<<endl
#define out2(a,b) cout<<#a<<" "<<a<<" "<<#b<<" "<<b<<endl
#define out3(a,b,c) cout<<#a<<" "<<a<<" "<<#b<<" "<<b<<" "<<#c<<" "<<c<<endl
#define out4(a,b,c,d) cout<<#a<<" "<<a<<" "<<#b<<" "<<b<<" "<<#c<<" "<<c<<" "<<#d<<" "<<d<<endl
#define fast ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
#define ft first
#define sd second
#define pb push_back
#define mp make_pair
#define endl "\n"
#define maxn 100001
#define THREAD_NUMBER 4
#define  DATA_SIZE 100

using namespace std;
using namespace std::chrono;



vector<float> err;

struct Point
{
    float x;
    float y;
    float z;

    int Cluster_no;
};



float distance(Point a, Point b)
{
    return sqrtf( powf((a.x - b.x),2) + powf((a.y - b.y),2) + powf((a.z - b.z),2) );
}

void get_data(vector<Point> &data) // dosyadan verileri aldığımız fonksiyon
{

    ifstream file;
    file.open("C:\\Users\\eyup\\CLionProjects\\kmeansproje\\data.csv");

    if(!file.is_open()) cout<<"Could not open file"<<endl;
    else cout<<"File open!"<<endl;

    string word,line,temp;
    getline(file,line);
    cout<<line<<endl;

    while(getline(file,line))
    {
        stringstream s(line);
        int cnt=0;
        Point a = {0,0,0,-1};
        while(getline(s,word,','))
        {
            if(cnt == 0) a.x = stof(word);
            if(cnt == 1) a.y = stof(word);
            if(cnt == 2) a.z = stof(word);

            cnt++;
        }
        data.push_back(a); // alınan veriler data'nın en sonuna aktarılıyor.
    }
}

void createData(vector<Point> &data)
{


    int minNumber = 0, maxNumber = 0;


    for (int i = 0; i < DATA_SIZE; ++i) {

        Point a = {0,0,0,-1};

        minNumber = 18, maxNumber = 80; // for age datas
        a.x = rand() % (maxNumber - minNumber + 1) + minNumber ;

        minNumber = 10, maxNumber = 150; // for income datas
        a.y = rand() % (maxNumber - minNumber + 1) + minNumber ;

        minNumber = 1, maxNumber = 100; // for spending score datas
        a.z = rand() % (maxNumber - minNumber + 1) + minNumber ;

        data.push_back(a);



    }
}

float SSE(vector<Point> &data,vector<Point> &cluster)
{
    float sse=0;
#pragma omp parallel for num_threads(THREAD_NUMBER)   // make parallel processing for SSE, i and data size are shared veriables
        rep(i,0,data.size())
        {

            if(data[i].Cluster_no!=-1)	sse += powf(distance(data[i],cluster[data[i].Cluster_no]),2);
        }





    return sse;
}

void put_data(vector<Point> &data,int K)
{
    ofstream file;
    file.open("data.csv");

    rep(i,0,data.size())
    {
        file<<data[i].x<<", "<<data[i].y<<", "<<data[i].z<<", "<<data[i].Cluster_no<<"\n";
    }
}



void standardize(vector<Point> &data)
{
    float mean_x,sd_x,mean_y,sd_y,mean_z,sd_z;

    mean_x = 0;mean_y = 0;mean_z = 0;
    sd_x=0;sd_y=0;sd_z=0;

    //Calculatung Mean
#pragma omp parallel for num_threads(THREAD_NUMBER)
    rep(i, 0, data.size()) {
        mean_x += data[i].x;
        mean_y += data[i].y;
        mean_z += data[i].z;
    }
    mean_x /= data.size();
    mean_y /= data.size();
    mean_z /= data.size();

    //Calculating S.D
#pragma omp parallel for num_threads(THREAD_NUMBER)
    rep(i,0,data.size())
    {
        sd_x += powf((data[i].x - mean_x),2);
        sd_y += powf((data[i].y - mean_y),2);
        sd_z += powf((data[i].z - mean_z),2);
    }
    sd_x /= data.size();
    sd_y /= data.size();
    sd_z /= data.size();

    sd_x=sqrtf(sd_x);
    sd_y=sqrtf(sd_y);
    sd_z=sqrtf(sd_z);

    //Standardize
#pragma omp parallel for num_threads(THREAD_NUMBER)
    rep(i,0,data.size())
    {
        data[i].x = (data[i].x - mean_x)/sd_x;
        data[i].y = (data[i].y - mean_y)/sd_y;
        data[i].z = (data[i].z - mean_z)/sd_z;
    }
}


void initialize_clustera(int K, vector<Point> &cluster, vector<Point> &data)
{
    unordered_map<int,int> index_visited;
    int cnt=0;
#pragma omp parallel for shared(cluster, data, index_visited, cnt) num_threads(THREAD_NUMBER)
    for (int i = 0; i < K; i++) {
        int z;
#pragma omp critical
        {
            z = rand() % data.size();
            if (index_visited[z] != 0) {
                i--;
                continue;
            }
            index_visited[z] = 1;
            cluster[i] = data[z];
            cnt++;
        }
    }
}


void assign_clusters(vector<Point> &cluster, vector<Point> &data)
{
#pragma omp parallel for num_threads(THREAD_NUMBER)
    rep(i,0,data.size())
    {
        float min_dist = FLT_MAX;

            rep(j,0,cluster.size())
            {
                float dist = distance(data[i], cluster[j]);
                if(dist < min_dist)
                {
#pragma omp critical
                    {
                        min_dist=dist;
                        data[i].Cluster_no = j;
                    }

                }
            }
        }


}

void recalculate_clusters(vector<Point> &cluster, vector<Point> &data,int K)
{
    Point temp={0,0,0,-1};
    temp.x=0;
    temp.y=0;
    temp.z=0;
    vector<Point> cluster_sum(K,temp);
    vector<int> cluster_num(K,0);

 #pragma omp parallel for num_threads(THREAD_NUMBER)

        rep(i,0,data.size())
        {
            if(data[i].Cluster_no != -1)
            {
                cluster_sum[data[i].Cluster_no].x += data[i].x;
                cluster_sum[data[i].Cluster_no].y += data[i].y;
                cluster_sum[data[i].Cluster_no].z += data[i].z;
                cluster_num[data[i].Cluster_no]++;
            }
        }
#pragma omp parallel for num_threads(THREAD_NUMBER)
        rep(i,0,K)
        {
            if(cluster_num[i]!=0)
            {
                cluster[i].x = cluster_sum[i].x / cluster_num[i];
                cluster[i].y = cluster_sum[i].y / cluster_num[i];
                cluster[i].z = cluster_sum[i].z / cluster_num[i];
            }
        }



}


bool is_different(vector<Point> &cluster,vector<Point> &previous_cluster,float error_limit)
{
    bool not_valid=false;

#pragma omp parallel for num_threads(THREAD_NUMBER)
        for(int i=0;i<cluster.size();i++)
        {
            if(cluster[i].x - previous_cluster[i].x > error_limit)
            {
                not_valid=true;
            }
            if(cluster[i].y - previous_cluster[i].y > error_limit)
            {
                not_valid=true;
            }
            if(cluster[i].z - previous_cluster[i].z > error_limit)
            {
                not_valid=true;
            }
        }


    return not_valid;
}


signed main()
{

    int K = 4;

    //This is to store all the points in dataset
    vector<Point> data;
    get_data(data);
    //createData(data);


    auto beg = high_resolution_clock::now();


    //As the data is in different scales its necessary to standardize it
    standardize(data);

    cout<<"Lets take a look at standardized data : "<<endl;
    rep(i,0,5)
    {
        cout<<data[i].x<<" "<<data[i].y<<" "<<data[i].z<<endl;
    }
    cout<<endl;

    //Initially each point dosent belong to any cluster
    rep(i,0,data.size())
    {
        data[i].Cluster_no=-1;
    }

    //stores the Cluster Centroids
    vector<Point> cluster(K,{0,0,0,-1});
    //We will first randomly select clusters from data
    initialize_clustera(K,cluster,data);

    cout<<"Initial values: "<<endl;
    rep(i,0,K)
    {
        cout<<cluster[i].x<<","<<cluster[i].y<<","<<cluster[i].z<<endl;
    }
    cout<<endl;

    //Stores the cluster centroids as they are later updated
    vector<Point> previous_cluster(K,{0,0,0,-1});

    //Do try other error limits
    float error_limit = 0.000001;

    while(is_different(cluster,previous_cluster,error_limit))
    {
        assign_clusters(cluster,data);

        rep(i,0,K)
        {
            previous_cluster[i] = cluster[i];
        }

        recalculate_clusters(cluster,data,K);
    }

    assign_clusters(cluster,data);

    //Final centroid positions
    cout<<"FINAL: "<<endl;
    rep(i,0,K)
    {
        cout<<cluster[i].x<<" "<<cluster[i].y<<" "<<cluster[i].z<<endl;
    }
    cout<<endl;

    put_data(data,K);
    cout<<"SSE : "<<SSE(data,cluster)<<endl;



    auto end = high_resolution_clock::now();

    // Subtracting the end timestamp from the beginning
    // And we choose to receive the difference in
    // microseconds
    auto duration = duration_cast<milliseconds>(end - beg);

    // Displaying the elapsed time
    std::cout << "Elapsed Time: (milliseconds)" << duration.count()<<endl;
    return 0;




}
