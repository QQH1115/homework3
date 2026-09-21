#include<iostream>
using ll=long long ;
// 打印用法帮助
void wrong(){
    std::cout << "用法: Myapp.exe -n <数量> -r <范围>\n      Myapp.exe -e <题目文件> -a <答案文件>\n";
}
int main(int argc,char*argv[]){
    ll o=0,n,r;      // o:模式 n:数量 r:范围
    string a,e;      // a:答案文件 e:题目文件
    if(argc<4){      // 参数太少
        wrong();
        return 1;
    }
    if(argv[1]=="-n"&&argv[3]=="-r")   // -n 数量 -r 范围
    {
        o=1;
        n=std::stoll(argv[2]);
        r=std::stoll(argv[4]);
    }
    else if(argv[1]=="-r"&&argv[3]=="-n")  // -r 范围 -n 数量
    {
        o=1;
        r=std::stoll(argv[2]);
        n=std::stoll(argv[4]);
    }
    else if(argv[1]=="-e"&&argv[3]=="-a")  // -e 题目 -a 答案
    {
        o=2;
        a=argv[4];
        e=argv[2];
    }
    else if(argv[1]=="-a"&&argv[3]=="-e")  // -a 答案 -e 题目
    {
        o=2;
        a=argv[2];
        e=argv[4];
    }
    else{            // 参数不合法
        wrong();
        return 1;
    }
    return 0;
}