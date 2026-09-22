#include<iostream>
#include<string>
#include<random>
#include <numeric>
#include <stack>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <charconv>   // 新增：to_chars / from_chars
using ll = long long;
std::mt19937_64 gen(std::random_device{}());
// 打印用法帮助
void wrong() {
    std::cout << "用法: Myapp.exe -n <数量> -r <范围>\n"
        "      Myapp.exe -e <题目文件> -a <答案文件>\n";
}
// 生成一道四则运算题目
// 参数 r: 数值范围，分子分母都在 [1, r] 之间随机取
// 返回: 一道题目字符串，例如 "3/4+2/3" 或 "(1/2+3)*4/5"
std::string generate(ll r) {
    std::string problem;      // 存放生成的题目
    problem.reserve(64);      // 新增：预留空间，减少重新分配

    // 随机决定这道题有几个操作数（1~4 个）
    // s 记录当前未匹配的左括号数量
    std::uniform_int_distribution<ll> dist14(1, 4);   // 新增：分布对象提到循环外
    std::uniform_int_distribution<ll> dist1r(1, r);   // 新增：分布对象提到循环外
    ll num = dist14(gen), s = 0;
    char buf[32];   // 新增：数字转字符串的缓冲区
    // 逐个生成操作数和运算符
    for (ll i = 0; i < num; i++) {
        ll w = 0;   // 标记当前操作数前面是否加了左括号
        // 如果不是最后一个操作数，随机决定是否加左括号
        // op==1 时加左括号，s 计数加一
        if (i != num - 1) {
            ll op = dist14(gen);   // 新增：改用循环外的分布对象
            if (op == 1) {
                problem += "(";
                s++;
                w = 1;
            }
        }
        // 随机生成分子 a 和分母 b，范围都是 [1, r]
        ll a = dist1r(gen);   // 新增：改用循环外的分布对象
        ll b = dist1r(gen);   // 新增：改用循环外的分布对象
        // 约分：分子分母同除以最大公约数
        ll g = std::gcd(a, b);
        a /= g;
        b /= g;
        // 如果分母为 1，就写成整数；否则写成分数 a/b
        if (b == 1) {
            auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), a);   // 新增：to_chars 替代 to_string
            problem.append(buf, p);                                    // 新增：直接追加缓冲区
        }
        else {
            auto [p1, ec1] = std::to_chars(buf, buf + sizeof(buf), a); // 新增：to_chars 替代 to_string
            problem.append(buf, p1);                                   // 新增：直接追加缓冲区
            problem += '/';
            auto [p2, ec2] = std::to_chars(buf, buf + sizeof(buf), b); // 新增：to_chars 替代 to_string
            problem.append(buf, p2);                                   // 新增：直接追加缓冲区
        }
        // 如果不是第一个操作数，且之前有未匹配的左括号，且当前操作数没加左括号
        // 则随机决定是否加右括号，s 计数减一
        if (i != 0 && s > 0 && w == 0) {
            ll op = dist14(gen);   // 新增：改用循环外的分布对象
            if (op == 1) {
                problem += ")";
                s--;
            }
        }
        // 如果不是最后一个操作数，随机生成一个运算符
        if (i != num - 1) {
            ll op = dist14(gen);   // 新增：改用循环外的分布对象
            switch (op) {
            case 1: problem += "+"; break;   // 加
            case 2: problem += "-"; break;   // 减
            case 3: problem += "*"; break;   // 乘
            case 4: problem += "÷"; break;   // 除（Unicode 字符，与分数里的 / 区分）
            }
        }
        else {
            // 最后一个操作数后，补全所有未匹配的左括号
            while (s > 0) {
                problem += ")";
                s--;
            }
        }
    }
    return problem;   // 返回生成的题目
}

// ========== 分数结构体 ==========
struct Fraction {
    ll num;
    ll den;
    bool isNegative() const {
        return num < 0;
    }
    Fraction(ll n = 0, ll d = 1) : num(n), den(d) {
        if (den < 0) { num = -num; den = -den; }
        reduce();
    }

    void reduce() {
        if (den == 0) throw std::runtime_error("分母为零");
        ll g = std::gcd(std::abs(num), den);
        num /= g;
        den /= g;
    }

    Fraction operator+(const Fraction& o) const {
        return Fraction(num * o.den + o.num * den, den * o.den);
    }
    Fraction operator-(const Fraction& o) const {
        return Fraction(num * o.den - o.num * den, den * o.den);
    }
    Fraction operator*(const Fraction& o) const {
        return Fraction(num * o.num, den * o.den);
    }
    Fraction operator/(const Fraction& o) const {
        if (o.num == 0) throw std::runtime_error("除以零");
        return Fraction(num * o.den, den * o.num);
    }

    std::string toString() const {
        if (den == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(den);
    }
};

// ========== 运算符优先级 ==========
int priority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '%') return 2;              // ÷ 的优先级
    if (op == '*' || op == '/') return 3; // / 比 ÷ 高，保证 ÷ 后的分数不被拆开
    return 0;
}
// ========== 判断是否是运算符 ==========
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

// ========== 中缀转后缀（调度场算法） ==========
std::vector<std::string> infixToPostfix(const std::string& expr) {
    std::vector<std::string> output;
    std::stack<char> ops;

    size_t i = 0;
    while (i < expr.size()) {
        char c = expr[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { i++; continue; }   // 新增：手写空白判断
        // 数字（含分数形式 a/b，整体读入，避免把分数的 / 当成除号）
        if (c >= '0' && c <= '9') {   // 新增：手写数字判断
            std::string num;
            while (i < expr.size() && ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '/')) {   // 新增：手写数字判断
                num += expr[i];
                i++;
            }
            output.push_back(num);
            continue;
        }
        // 左括号
        if (c == '(') {
            ops.push(c);
            i++;
            continue;
        }
        // 右括号
        if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                output.push_back(std::string(1, ops.top()));
                ops.pop();
            }
            if (ops.empty()) throw std::runtime_error("括号不匹配");
            ops.pop();
            i++;
            continue;
        }
        // 除号 ÷（UTF-8 两字节）→ 转成 %，与分数里的 / 区分
        if ((unsigned char)c == 0xC3 && i + 1 < expr.size() &&
            (unsigned char)expr[i + 1] == 0xB7) {
            while (!ops.empty() && priority(ops.top()) >= priority('%')) {
                output.push_back(std::string(1, ops.top()));
                ops.pop();
            }
            ops.push('%');
            i += 2;
            continue;
        }
        // 普通运算符（+ - * /）
        if (isOperator(c)) {
            while (!ops.empty() && priority(ops.top()) >= priority(c)) {
                output.push_back(std::string(1, ops.top()));
                ops.pop();
            }
            ops.push(c);
            i++;
            continue;
        }
        throw std::runtime_error(std::string("非法字符: ") + c);
    }
    while (!ops.empty()) {
        if (ops.top() == '(') throw std::runtime_error("括号不匹配");
        output.push_back(std::string(1, ops.top()));
        ops.pop();
    }
    return output;
}
// ========== 后缀表达式求值（分数） ==========
Fraction evalPostfix(const std::vector<std::string>& postfix) {
    std::stack<Fraction> st;
    for (const auto& token : postfix) {
        // 运算符
        if (token.size() == 1 && isOperator(token[0])) {
            if (st.size() < 2) throw std::runtime_error("表达式非法");
            Fraction b = st.top(); st.pop();
            Fraction a = st.top(); st.pop();
            char op = token[0];

            Fraction r;
            if (op == '+')      r = a + b;
            else if (op == '-') r = a - b;
            else if (op == '*') r = a * b;
            else if (op == '/' || op == '%') r = a / b;   // / 和 % 都当除法

            // 任何一步出现负数，就抛异常，让上层作废重生成
            if (r.isNegative()) {
                throw std::runtime_error("出现负数");
            }

            st.push(r);
        }
        else {
            // 数字或分数（按第一个 / 拆成分子分母）
            size_t slash = token.find('/');
            if (slash == std::string::npos) {
                ll n;
                std::from_chars(token.data(), token.data() + token.size(), n);   // 新增：from_chars 替代 stoll
                st.push(Fraction(n, 1));
            }
            else {
                ll n, d;
                std::from_chars(token.data(), token.data() + slash, n);                            // 新增：from_chars 替代 stoll
                std::from_chars(token.data() + slash + 1, token.data() + token.size(), d);          // 新增：from_chars 替代 stoll
                st.push(Fraction(n, d));
            }
        }
    }
    if (st.size() != 1) throw std::runtime_error("表达式非法");
    return st.top();
}
// ========== 对外接口：计算表达式 ==========
// 输入: "3+4*2" 或 "(1+3)*4/5" 或 "3÷4"
// 输出: 结果字符串，例如 "17/12"
std::string calculate(const std::string& expr) {
    auto postfix = infixToPostfix(expr);
    Fraction result = evalPostfix(postfix);
    return result.toString();
}
void filewrite(const std::string& filename, const std::string& expr) {
    std::ofstream fout(filename, std::ofstream::app);
    fout << expr;
    fout.close();
}
int main(int argc, char* argv[]) {
    ll o = 0, n, r;      // o:模式 n:数量 r:范围
    std::string a, e, argv1, argv3;      // a:答案文件 e:题目文件
    if (argc != 5) {      // 参数太少
        wrong();
        return 1;
    }
    argv1 = argv[1];
    argv3 = argv[3];
    if (argv1 == "-n" && argv3 == "-r")   // -n 数量 -r 范围
    {
        o = 1;
        n = std::stoll(argv[2]);
        r = std::stoll(argv[4]);
    }
    else if (argv1 == "-r" && argv3 == "-n")  // -r 范围 -n 数量
    {
        o = 1;
        r = std::stoll(argv[2]);
        n = std::stoll(argv[4]);
    }
    else if (argv1 == "-e" && argv3 == "-a")  // -e 题目 -a 答案
    {
        o = 2;
        a = argv[4];
        e = argv[2];
    }
    else if (argv1 == "-a" && argv3 == "-e")  // -a 答案 -e 题目
    {
        o = 2;
        a = argv[2];
        e = argv[4];
    }
    else {            // 参数不合法
        wrong();
        return 1;
    }
    if (o == 1) {
        std::ofstream foutEx("Exercises.txt", std::ofstream::trunc);   // 新增：文件流提到循环外
        std::ofstream foutAns("Answers.txt", std::ofstream::trunc);    // 新增：文件流提到循环外
        std::unordered_map<std::string, ll> p;
        ll s = 0;
        char idxBuf[32];   // 新增：编号转字符串的缓冲区
        for (ll i = 0; i < n; i++) {
            std::string problem = generate(r);
            // 声明一个字符串，用来存放计算出来的答案
            std::string expr;

            // 计算这道题的答案
            // calculate 可能抛异常（比如除以零、括号不匹配），所以要用 try 包起来
            try {
                expr = calculate(problem);
            }
            catch (const std::exception& ex) {
                // 计算失败，说明这道题非法（比如分母为 0）
                // i-- 让 for 循环的 i++ 抵消，相当于重新生成这一题
                i--;
                s++;
                continue;
            }
            // 判断这个答案是否已经出现过（答案去重）
            // p 的 key 是答案字符串，value 随便存个 1 表示出现过
            if (p.find(expr) != p.end()) {
                // 答案重复，这一题作废
                i--;          // 让 for 的 i++ 抵消，重新生成这一题
                s++;          // 重复次数加一

                // 如果重复次数太多，说明范围 r 太小，凑不出 n 道不重复的题
                // 直接退出程序，避免无限循环
                if (s > n * 10000) {
                    return 1;
                }
                continue;     // 跳过本轮剩下的写入操作
            }
            else {
                // 答案没出现过，记录下来，防止后面再生成同样的答案
                p[expr] = 1;
                auto [p, ec] = std::to_chars(idxBuf, idxBuf + sizeof(idxBuf), i + 1);   // 新增：编号转字符串
                // 把题目写入 Exercises.txt
                // 格式: "编号.题目"，例如 "1.3/4+2/3"
                foutEx.write(idxBuf, p - idxBuf);                  // 新增：直接写入缓冲区
                foutEx.put('.');
                foutEx.write(problem.data(), problem.size());
                foutEx.put('\n');
                // 把答案写入 Answers.txt
                // 格式: "编号.答案"，例如 "1.17/12"
                foutAns.write(idxBuf, p - idxBuf);                 // 新增：直接写入缓冲区
                foutAns.put('.');
                foutAns.write(expr.data(), expr.size());
                foutAns.put('\n');
            }
        }
    }
    else {
        // ========== 批改：对比题目和答案，生成 Grade.txt ==========
        std::ifstream finEx(e);       // 用命令行传入的题目文件
        std::ifstream finAns(a);      // 用命令行传入的答案文件
        if (!finEx || !finAns) {
            std::cerr << "无法打开文件" << std::endl;
        }
        else {
            std::vector<ll> correct, wrong;   // 做对、做错的题号
            std::string lineEx, lineAns;

            while (std::getline(finEx, lineEx) && std::getline(finAns, lineAns)) {

                // 解析题目行: "1.3/4+2/3"
                size_t dotEx = lineEx.find('.');
                ll idx = std::stoll(lineEx.substr(0, dotEx));       // 题号
                std::string expr = lineEx.substr(dotEx + 1);        // 表达式

                // 解析答案行: "1.17/12"
                size_t dotAns = lineAns.find('.');
                std::string stuAns = lineAns.substr(dotAns + 1);    // 学生答案
                // 计算正确答案（题目保证合法，不会抛异常）
                std::string correctAns = calculate(expr);
                // 对比
                if (stuAns == correctAns) {
                    correct.push_back(idx);
                }
                else {
                    wrong.push_back(idx);
                }
            }
            // 清空 Grade.txt
            std::ofstream("Grade.txt", std::ofstream::trunc).close();
            // Correct 行
            std::string line = "Correct: " + std::to_string(correct.size()) + " (";
            for (size_t i = 0; i < correct.size(); i++) {
                if (i > 0) line += ", ";
                line += std::to_string(correct[i]);
            }
            line += ")\n";
            filewrite("Grade.txt", line);
            // Wrong 行
            line = "Wrong: " + std::to_string(wrong.size()) + " (";
            for (size_t i = 0; i < wrong.size(); i++) {
                if (i > 0) line += ", ";
                line += std::to_string(wrong[i]);
            }
            line += ")\n";
            filewrite("Grade.txt", line);
        }
    }
    return 0;
}