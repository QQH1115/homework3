#include<iostream>
#include<string>
#include<random>
#include <numeric>
using ll=long long ;
std::mt19937_64 gen(std::random_device{}());
// 打印用法帮助
void wrong(){
    std::cout << "用法: Myapp.exe -n <数量> -r <范围>\n      Myapp.exe -e <题目文件> -a <答案文件>\n";
}
// 生成一道四则运算题目
// 参数 r: 数值范围，分子分母都在 [1, r] 之间随机取
// 返回: 一道题目字符串，例如 "3/4+2/3" 或 "(1/2+3)*4/5"
std::string generate(ll r) {
    std::string problem;      // 存放生成的题目
    problem.clear();          // 清空字符串（其实新建时本来就是空的，可省略）

    // 随机决定这道题有几个操作数（1~4 个）
    // s 记录当前未匹配的左括号数量
    ll num = std::uniform_int_distribution<ll>(1, 4)(gen), s = 0;

    // 逐个生成操作数和运算符
    for (ll i = 0; i < num; i++) {
        ll w = 0;   // 标记当前操作数前面是否加了左括号

        // 如果不是最后一个操作数，随机决定是否加左括号
        // op==1 时加左括号，s 计数加一
        if (i != num - 1) {
            ll op = std::uniform_int_distribution<ll>(1, 4)(gen);
            if (op == 1) {
                problem += "(";
                s++;
                w = 1;
            }
        }

        // 随机生成分子 a 和分母 b，范围都是 [1, r]
        ll a = std::uniform_int_distribution<ll>(1, r)(gen);
        ll b = std::uniform_int_distribution<ll>(1, r)(gen);

        // 约分：分子分母同除以最大公约数
        ll g = std::gcd(a, b);
        a /= g;
        b /= g;

        // 如果分母为 1，就写成整数；否则写成分数 a/b
        if (b == 1) {
            problem += std::to_string(a);
        } else {
            problem += std::to_string(a) + "/" + std::to_string(b);
        }

        // 如果不是第一个操作数，且之前有未匹配的左括号，且当前操作数没加左括号
        // 则随机决定是否加右括号，s 计数减一
        if (i != 0 && s > 0 && w == 0) {
            ll op = std::uniform_int_distribution<ll>(1, 4)(gen);
            if (op == 1) {
                problem += ")";
                s--;
            }
        }

        // 如果不是最后一个操作数，随机生成一个运算符
        if (i != num - 1) {
            ll op = std::uniform_int_distribution<ll>(1, 4)(gen);
            switch (op) {
                case 1: problem += "+"; break;   // 加
                case 2: problem += "-"; break;   // 减
                case 3: problem += "*"; break;   // 乘
                case 4: problem += "÷"; break;   // 除（Unicode 字符）
            }
        } else {
            // 最后一个操作数后，补全所有未匹配的左括号
            while (s > 0) {
                problem += ")";
                s--;
            }
        }
    }

    return problem;   // 返回生成的题目
}
int main(int argc,char*argv[]){
    ll o=0,n,r;      // o:模式 n:数量 r:范围
    std::string a,e;      // a:答案文件 e:题目文件
    if(argc<4||argc>4){      // 参数太少
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
    if(o==1){
        for(ll i=0;i<n;i++){
            std::string problem=generate(r);
        }
    }
    else{

    }
    return 0;
}