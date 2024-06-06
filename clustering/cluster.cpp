#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

/*
 * format: traceID,order,structtype,method1,method2,m1_para,m2_para,class1,class2,m1_return,m2_return
 */

struct TraceInfo
{
    int traceID;
    int order;
    std::string structtype;
    std::string method1;
    std::string method2;
    std::string param1;
    std::string param2;
    std::string class1;
    std::string class2;
    std::string m1_return;
    std::string m2_return;
};

void PrintTraceInfo(TraceInfo &info)
{
    std::cout << "traceID: " << info.traceID << " order: " << info.order << " structtype: " << info.structtype << " method1: " << info.method1 << " method2: " << info.method2 << " param1: " << info.param1
              << " param2: " << info.param2 << " class1: " << info.class1 << " class2: " << info.class2 << " m1_return: " << info.m1_return
              << " m2_return: " << info.m2_return << std::endl;
}

void parseDoc(std::vector<TraceInfo> &vec, const char *filename)
{
    std::fstream input(filename, std::fstream::in);
    TraceInfo info;

    while (input)
    {
        std::string s;
        input >> s;

        for (int i = 0, id = 0; i < s.size(); ++i)
        {
            int j = i;
            while (j < s.size() && s[j] != ',')
                ++j;

            switch (id)
            {
            case 0:
                info.traceID = std::stoi(s.substr(i, j - i));
                break;

            case 1:
                info.order = std::stoi(s.substr(i, j - i));
                break;

            case 2:
                info.structtype = s.substr(i, j - i);
                break;

            case 3:
                info.method1 = s.substr(i, j - i);
                break;

            case 4:
                info.method2 = s.substr(i, j - i);
                break;

            case 5:
                info.param1 = s.substr(i, j - i);
                break;

            case 6:
                info.param2 = s.substr(i, j - i);
                break;

            case 7:
                info.class1 = s.substr(i, j - i);
                break;

            case 8:
                info.class2 = s.substr(i, j - i);
                break;

            case 9:
                info.m1_return = s.substr(i, j - i);
                break;

            case 10:
                info.m2_return = s.substr(i, j - i);
                break;

            default:
                break;
            }

            ++id;
            i = j;
        }

        vec.push_back(info);
    }
}

void PrintCluster(std::unordered_set<std::string> &cluster)
{
    for (auto it = cluster.begin(); it != cluster.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void PrintClusters(std::vector<std::unordered_set<std::string>> &clusters)
{
    std::cout << "Total: " << clusters.size() << std::endl;
    for (int i = 0; i < clusters.size(); ++i)
    {
        std::cout << "--------------------Cluster " << i + 1 << "--------------------" << std::endl;
        PrintCluster(clusters[i]);
    }
}

void dfs(int u, std::vector<std::vector<int>> &graph, std::vector<std::unordered_set<int>> &cluster_ids, std::unordered_set<int> &path_ids, std::vector<bool> &mark)
{
    path_ids.insert(u);

    bool is_end = true;
    for (auto x : graph[u])
    {
        if (!mark[x])
        {
            is_end = false;
            mark[x] = true;
            dfs(x, graph, cluster_ids, path_ids, mark);
            mark[x] = false;
        }
    }

    if (is_end)
    {
        cluster_ids.push_back(path_ids);
    }

    path_ids.erase(u);
}

void initClusters(std::vector<std::unordered_set<std::string>> &clusters, const std::vector<TraceInfo> &traceInfos)
{
    std::unordered_map<std::string, int> class_to_id;
    std::unordered_map<int, std::string> id_to_class;

    int cnt = 0;
    for (auto &traceInfo : traceInfos)
    {
        if (class_to_id.count(traceInfo.class1) == 0)
        {
            class_to_id[traceInfo.class1] = cnt;
            id_to_class[cnt++] = traceInfo.class1;
        }

        if (class_to_id.count(traceInfo.class2) == 0)
        {
            class_to_id[traceInfo.class2] = cnt;
            id_to_class[cnt++] = traceInfo.class2;
        }
    }

    std::vector<std::vector<int>> graph(cnt);
    std::vector<bool> is_root(cnt, true);

    for (auto &traceInfo : traceInfos)
    {
        int class1_id = class_to_id[traceInfo.class1];
        int class2_id = class_to_id[traceInfo.class2];

        if (class1_id != class2_id && std::lower_bound(graph[class1_id].begin(), graph[class1_id].end(), class2_id) == graph[class1_id].end())
        {
            is_root[class2_id] = false;
            graph[class1_id].push_back(class2_id);
        }
    }

    std::vector<std::unordered_set<int>> cluster_ids;
    for (int i = 0; i < cnt; ++i)
    {
        if (is_root[i])
        {
            std::unordered_set<int> path_ids;
            std::vector<bool> mark(cnt);
            dfs(i, graph, cluster_ids, path_ids, mark);
        }
    }

    for (auto &cluster_id : cluster_ids)
    {
        std::unordered_set<std::string> temp;
        for (auto it = cluster_id.begin(); it != cluster_id.end(); ++it)
        {
            temp.insert(id_to_class[*it]);
        }
        clusters.push_back(temp);
    }
};

int calcSim(const std::unordered_set<std::string> &cluster1, const std::unordered_set<std::string> &cluster2)
{

    std::unordered_set<std::string> temp;
    for (auto &x : cluster1)
        temp.insert(x);

    for (auto &x : cluster2)
    {
        temp.insert(x);
    }

    int unioncnt = temp.size(), intersec = 0;

    for (auto &x : temp)
    {
        if (cluster1.count(x) && cluster2.count(x))
            ++intersec;
    }

    return intersec / unioncnt;
}

void doClustering(std::vector<std::unordered_set<std::string>> &clusters, int K)
{
    int sz = clusters.size();
    while (sz > K)
    {
        int max_i = 0, max_j = 0, max_sim = 0;
        std::vector<std::vector<int>> sims(sz, std::vector<int>(sz));
        for (int i = 0; i < sz; ++i)
        {
            for (int j = 0; j < sz; ++j)
            {
                sims[i][j] = calcSim(clusters[i], clusters[j]);
                if (sims[i][j] > max_sim)
                {
                    max_sim = sims[i][j];
                    max_i = i, max_j = j;
                }
            }
        }

        for (auto &x : clusters[max_j])
        {
            clusters[max_i].insert(x);
        }
        std::swap(clusters[max_j], clusters.back());
        clusters.pop_back();
        --sz;
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "usage: ./cluster [filepath]" << std::endl;
        return 1;
    }

    std::vector<TraceInfo> traceInfos;
    parseDoc(traceInfos, argv[1]);

    std::vector<std::unordered_set<std::string>> clusters;
    initClusters(clusters, traceInfos);

    PrintClusters(clusters);
    doClustering(clusters, 3);
    PrintClusters(clusters);

    return 0;
}