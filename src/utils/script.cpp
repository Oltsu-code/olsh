#include "../../include/utils/script.h"
#include "../../include/shell.h"
#include <utils/colors.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>
#include <cctype>


namespace {
    static inline std::string trim(const std::string& s) {
        size_t start = 0; while (start < s.size() && std::isspace((unsigned char)s[start])) start++;
        size_t end = s.size(); while (end > start && std::isspace((unsigned char)s[end-1])) end--;
        return s.substr(start, end - start);
    }

    static inline bool starts_with(const std::string& s, const std::string& p) {
        return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
    }

    static inline bool ends_with(const std::string& s, const std::string& suf) {
        return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
    }

    static std::vector<std::string> split_words(const std::string& line) {
        std::vector<std::string> out;
        std::string cur;
        bool in_s=false, in_d=false;
        for (size_t i=0;i<line.size();++i) {
            char c=line[i];
            if (c=='\\' && i+1<line.size()) { cur.push_back(line[++i]); continue; }
            if (c=='"' && !in_s) { in_d=!in_d; continue; }
            if (c=='\'' && !in_d) { in_s=!in_s; continue; }
            if (!in_s && !in_d && std::isspace((unsigned char)c)) { if(!cur.empty()){ out.push_back(cur); cur.clear(); } }
            else cur.push_back(c);
        }
        if(!cur.empty()) out.push_back(cur);
        return out;
    }
}

namespace olsh::Utils {

ScriptInterpreter::ScriptInterpreter(olsh::Shell* shellInstance) : shell(shellInstance) {}

bool ScriptInterpreter::isScriptFile(const std::string& filename) {
    return filename.size() >= 5 && filename.substr(filename.size() - 5) == ".olsh";
}

int ScriptInterpreter::executeScript(const std::string& filename) {
    return executeScript(filename, {});
}

int ScriptInterpreter::executeScript(const std::string& filename,
                                     const std::vector<std::string>& args) {
    if (!isScriptFile(filename)) {
        std::cerr << RED << "Error: Not a valid .olsh script file: " << filename << RESET << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED << "Error: Cannot open script file: " << filename << RESET << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return executeScriptContent(buffer.str(), args);
}

int ScriptInterpreter::executeScriptContent(const std::string& content) {
    return executeScriptContent(content, {});
}

int ScriptInterpreter::executeScriptContent(const std::string& content,
                                            const std::vector<std::string>& args) {
    std::istringstream stream(content);
    return executeLines(stream, args, 0);
}

// ---- helpers ----

// simple arithmetic parser
class ArithParser {
    const std::string& s; size_t i;
    void skip(){ while(i<s.size() && std::isspace((unsigned char)s[i])) i++; }
    long long parseNumber(){ skip(); bool neg=false; if (i<s.size() && (s[i]=='+' || s[i]=='-')) { neg = s[i]=='-'; i++; }
        skip(); long long val=0; bool any=false; while(i<s.size() && std::isdigit((unsigned char)s[i])){ any=true; val = val*10 + (s[i]-'0'); i++; }
        return neg? -val: val; }
    long long parseFactor(){ skip(); if (i<s.size() && s[i]=='('){ i++; long long v=parseExpr(); skip(); if (i<s.size() && s[i]==')') i++; return v; } return parseNumber(); }
    long long parseTerm(){ long long v=parseFactor(); while(true){ skip(); if (i>=s.size()) break; char c=s[i]; if (c=='*'||c=='/'||c=='%'){ i++; long long r=parseFactor(); if(c=='*') v*=r; else if(c=='/') v = r==0? 0 : v/r; else v = r==0? 0 : v%r; } else break; } return v; }
public:
    ArithParser(const std::string& str): s(str), i(0){}
    long long parseExpr(){ long long v=parseTerm(); while(true){ skip(); if (i>=s.size()) break; char c=s[i]; if (c=='+'||c=='-'){ i++; long long r=parseTerm(); if(c=='+') v+=r; else v-=r; } else break; } return v; }
};

long long ScriptInterpreter::evalArithmetic(const std::string& expr) {
    ArithParser p(expr);
    return p.parseExpr();
}

std::string ScriptInterpreter::expandArithmetic(const std::string& line) {
    std::string out; out.reserve(line.size());
    for (size_t i=0;i<line.size();){
        if (line[i]=='$' && i+3<line.size() && line[i+1]=='(' && line[i+2]=='(') {
            size_t j=i+3; int depth=1; while(j<line.size() && depth>0){ if(line[j]=='(') depth++; else if(line[j]==')') depth--; j++; }
            if (depth==0 && j<line.size() && line[j]==')') { // had closing ))
                std::string expr = line.substr(i+3, (j-1)-(i+3));
                // replace bare identifiers with variable values
                std::string replaced; replaced.reserve(expr.size());
                for (size_t k=0; k<expr.size(); ){
                    char c = expr[k];
                    if (std::isalpha((unsigned char)c) || c=='_'){
                        size_t m=k+1; while(m<expr.size() && (std::isalnum((unsigned char)expr[m]) || expr[m]=='_')) m++;
                        std::string name = expr.substr(k, m-k);
                        std::string val;
                        auto it = variables.find(name);
                        if (it != variables.end()) val = it->second; else {
                            const char* env = std::getenv(name.c_str()); if (env) val = env; else val = "0";
                        }
                        // if val not numeric, default to 0
                        bool numeric=true; if (val.empty()) numeric=false; else {
                            size_t p=0; if (val[0]=='+'||val[0]=='-') p=1; for (; p<val.size(); ++p){ if(!std::isdigit((unsigned char)val[p])) { numeric=false; break; } }
                        }
                        replaced += numeric ? val : std::string("0");
                        k = m; continue;
                    }
                    replaced.push_back(c); k++;
                }
                long long v = evalArithmetic(replaced);
                out += std::to_string(v);
                i = j+1;
                continue;
            }
        }
        out.push_back(line[i++]);
    }
    return out;
}

std::string ScriptInterpreter::substituteBackticks(const std::string& line,
                                                   int lastExitCode,
                                                   const std::vector<std::string>& args) {
    std::string out; out.reserve(line.size());
    for (size_t i=0;i<line.size();){
        if (line[i]=='`'){
            size_t j=i+1; while(j<line.size() && line[j] != '`') j++;
            if (j<line.size() && line[j]=='`'){
                std::string cmd = line.substr(i+1, j-(i+1));
                // Expand inside backticks first (no nested backticks)
                cmd = expandLine(cmd, lastExitCode, args);
                // decide if builtin
                auto words = split_words(cmd);
                bool isBuiltin = !words.empty() && (
                    words[0]=="cd" || words[0]=="ls" || words[0]=="pwd" || words[0]=="echo" ||
                    words[0]=="rm" || words[0]=="cat" || words[0]=="clear" || words[0]=="history" || words[0]=="alias"
                );
                std::string val;
                if (isBuiltin) {
                    // capture stdout during execution
                    auto* old = std::cout.rdbuf();
                    std::ostringstream capture;
                    std::cout.rdbuf(capture.rdbuf());
                    (void)shell->processCommand(cmd);
                    std::cout.rdbuf(old);
                    val = capture.str();
                } else {
                    // capture external using popen
                    FILE* pipe = nullptr;
#ifdef _WIN32
                    pipe = _popen(cmd.c_str(), "r");
#else
                    pipe = popen(cmd.c_str(), "r");
#endif
                    if (pipe) {
                        char buffer[512]; size_t n;
                        while ((n = fread(buffer, 1, sizeof(buffer), pipe)) > 0) val.append(buffer, n);
#ifdef _WIN32
                        _pclose(pipe);
#else
                        pclose(pipe);
#endif
                    }
                }
                // trim trailing newlines
                while(!val.empty() && (val.back()=='\n' || val.back()=='\r')) val.pop_back();
                out += val;
                i = j+1;
                continue;
            }
        }
        out.push_back(line[i++]);
    }
    return out;
}

std::string ScriptInterpreter::expandVariables(const std::string& line,
                                               int lastExitCode,
                                               const std::vector<std::string>& args) {
    std::string out; out.reserve(line.size());
    for (size_t i=0;i<line.size();){
        if (line[i]=='$'){
            // special cases: $? $@ $# $1..$9 and $VAR
            if (i+1<line.size() && line[i+1]=='?') { out += std::to_string(lastExitCode); i+=2; continue; }
            if (i+1<line.size() && line[i+1]=='@') {
                std::string joined;
                for (size_t k=0;k<args.size();++k){ if(k) joined+=' '; joined+=args[k]; }
                out += joined; i+=2; continue;
            }
            if (i+1<line.size() && line[i+1]=='#') { out += std::to_string(args.size()); i+=2; continue; }
            // positional
            if (i+1<line.size() && std::isdigit((unsigned char)line[i+1])){
                size_t j=i+1; int idx = 0; while(j<line.size() && std::isdigit((unsigned char)line[j])){ idx = idx*10 + (line[j]-'0'); j++; }
                if (idx>=1 && (size_t)idx <= args.size()) out += args[idx-1];
                i = j; continue;
            }
            // ${VAR}
            if (i+2<line.size() && line[i+1]=='{' ){
                size_t j=i+2; while(j<line.size() && line[j] != '}') j++;
                if (j<line.size()){
                    std::string name = line.substr(i+2, j-(i+2));
                    std::string val;
                    auto it = variables.find(name);
                    if (it != variables.end()) val = it->second; else {
                        const char* env = std::getenv(name.c_str()); if(env) val = env;
                    }
                    out += val; i = j+1; continue;
                }
            }
            // $VAR_NAME
            size_t j=i+1; while(j<line.size() && (std::isalnum((unsigned char)line[j]) || line[j]=='_' )) j++;
            if (j>i+1){
                std::string name = line.substr(i+1, j-(i+1));
                std::string val;
                auto it = variables.find(name);
                if (it != variables.end()) val = it->second; else { const char* env = std::getenv(name.c_str()); if(env) val=env; }
                out += val; i = j; continue;
            }
        }
        out.push_back(line[i++]);
    }
    return out;
}

std::string ScriptInterpreter::expandLine(const std::string& input,
                                          int lastExitCode,
                                          const std::vector<std::string>& args) {
    // order: arithmetic, backticks, variables
    std::string s = expandArithmetic(input);
    s = substituteBackticks(s, lastExitCode, args);
    s = expandVariables(s, lastExitCode, args);
    return s;
}

bool ScriptInterpreter::evalCondition(const std::string& condRaw,
                                      int lastExitCode,
                                      const std::vector<std::string>& args) {
    std::string cond = trim(condRaw);
    if (starts_with(cond, "[")){
        // trim [ and ] and optional ; then
        if (cond.front()=='['){
            // remove leading '[' and trailing ']' (and optional ';')
            size_t l = cond.find('['); size_t r = cond.rfind(']');
            if (r!=std::string::npos && r>l) cond = trim(cond.substr(l+1, r-l-1));
        }
    }
    // tokens
    auto parts = split_words(cond);
    auto asInt = [](const std::string& s)->long long{ try{ return std::stoll(s);}catch(...){return 0;} };
    auto get = [&](size_t idx)->std::string{ return idx<parts.size()? parts[idx]: std::string(); };
    if (parts.size()==1){ return !get(0).empty(); }
    if (parts.size()==2){
        std::string op = get(0), arg = get(1);
        if (op=="-z") return arg.empty();
        if (op=="-n") return !arg.empty();
    }
    if (parts.size()==3){
        std::string a = get(0), op = get(1), b = get(2);
        if (op=="-eq") return asInt(a)==asInt(b);
        if (op=="-ne") return asInt(a)!=asInt(b);
        if (op=="-lt") return asInt(a)<asInt(b);
        if (op=="-le") return asInt(a)<=asInt(b);
        if (op=="-gt") return asInt(a)>asInt(b);
        if (op=="-ge") return asInt(a)>=asInt(b);
        if (op=="=") return a==b;
        if (op!="=") return a!=b;
    }
    // default false
    return false;
}

// core executor for a set of lines
int ScriptInterpreter::executeBlock(const std::vector<std::string>& lines,
                                    std::vector<std::string> args,
                                    int depth) {
    int lastExitCode = 0;
    for (size_t idx=0; idx<lines.size(); ++idx){
        std::string raw = trim(lines[idx]);
        if (raw.empty()) continue;
        if (raw[0]=='#') continue;

        // function definition
        if (starts_with(raw, "function ")){
            auto rest = trim(raw.substr(9));
            // format: name {  or name{ on same line
            std::string name; size_t pos = rest.find('{');
            if (pos != std::string::npos){ name = trim(rest.substr(0,pos)); }
            else { name = trim(rest); }
            // collect body until closing '}'
            std::vector<std::string> body;
            if (pos != std::string::npos && pos+1 < rest.size()){
                std::string after = trim(rest.substr(pos+1));
                if (!after.empty()) body.push_back(after);
            }
            int brace=1;
            while (++idx < lines.size()){
                std::string ln = lines[idx];
                for(char c: ln){ if(c=='{') brace++; else if(c=='}') brace--; }
                // strip trailing '}' if present
                std::string lntrim = ln;
                if (!lntrim.empty() && lntrim.back()=='}'){
                    // remove last closing brace
                    size_t cut = lntrim.find_last_of('}');
                    lntrim = trim(lntrim.substr(0, cut));
                }
                if (!lntrim.empty()) body.push_back(lntrim);
                if (brace==0) break;
            }
            functions[name] = FunctionDef{body};
            continue;
        }

        // expand line for substitution before control structure parsing
        std::string line = expandLine(raw, lastExitCode, args);

        // set builtin: set VAR = value
        if (starts_with(line, "set ")){
            std::string rest = trim(line.substr(4));
            size_t eq = rest.find('=');
            if (eq != std::string::npos){
                std::string var = trim(rest.substr(0, eq));
                std::string val = trim(rest.substr(eq+1));
                variables[var] = val;
            }
            continue;
        }

        // if/elif/else/fi handling: collect whole if block starting at current idx
        if (starts_with(raw, "if ") || starts_with(raw, "if[")){
            // accumulate lines until 'fi'
            std::vector<std::pair<std::string, std::vector<std::string>>> branches; // (cond or "else", body)
            std::string currentCond = trim(raw);
            std::vector<std::string> currentBody;
            bool inThen = false;
            // If line contains 'then', split
            auto handleThenSplit = [&](const std::string& l){
                size_t th = l.find("then");
                if (th != std::string::npos){
                    inThen = true; return trim(l.substr(th+4));
                }
                return std::string();
            };
            std::string remainder = handleThenSplit(raw);
            if (!remainder.empty()) currentBody.push_back(remainder);
            while (++idx < lines.size()){
                std::string l = trim(lines[idx]);
                if (l == "fi") break;
                if (starts_with(l, "elif ")){
                    // push previous branch
                    branches.push_back({currentCond, currentBody});
                    currentCond = l; currentBody.clear(); inThen=false;
                    remainder = handleThenSplit(l);
                    if (!remainder.empty()) currentBody.push_back(remainder);
                    continue;
                }
                if (l == "else"){
                    branches.push_back({currentCond, currentBody});
                    currentCond = "else"; currentBody.clear(); inThen=true; continue;
                }
                std::string r = l;
                if (!inThen){ r = handleThenSplit(l); if (r.empty()) continue; }
                currentBody.push_back(r);
            }
            branches.push_back({currentCond, currentBody});

            // evaluate branches
            bool executed=false; int rc=0;
            for (auto& [condLine, body] : branches){
                if (condLine=="else"){
                    rc = executeBlock(body, args, depth+1); executed=true; break;
                }
                // condition text between if and then
                std::string condText = condLine;
                if (starts_with(condText, "if ")) condText = trim(condText.substr(3));
                // remove trailing ; if any
                if (!condText.empty() && condText.back()==';') condText.pop_back();
                // expand and evaluate
                condText = expandLine(condText, lastExitCode, args);
                if (evalCondition(condText, lastExitCode, args)){
                    rc = executeBlock(body, args, depth+1); executed=true; break;
                }
            }
            lastExitCode = rc;
            continue;
        }

        // while loop: while [ condition ]; do ... done
        if (starts_with(raw, "while ")){
            // gather body until done
            std::string header = raw; // may include ; do
            std::vector<std::string> body;
            bool haveDo = header.find(" do") != std::string::npos || ends_with(header, ";do") || ends_with(header, "; do");
            if (haveDo){
                size_t p = header.find("do");
                if (p!=std::string::npos){ std::string after = trim(header.substr(p+2)); if(!after.empty()) body.push_back(after); header = trim(header.substr(0,p)); }
            } else {
                // scan lines to find a 'do' line; leave idx on that line
                while (idx + 1 < lines.size()){
                    std::string l = trim(lines[idx+1]);
                    if (l == "do") { idx += 1; haveDo = true; break; }
                    idx += 1; // skip intervening lines if any (unlikely)
                }
            }
            // now collect body until done, starting after current idx
            while (idx + 1 < lines.size()){
                std::string l = lines[idx+1];
                if (trim(l) == "done") { idx += 1; break; }
                body.push_back(l);
                idx += 1;
            }
            // condition text between while and (optional) ; do
            std::string condText = trim(header.substr(6));
            int rc = 0;
            while (true){
                std::string expandedCond = expandLine(condText, lastExitCode, args);
                if (!evalCondition(expandedCond, lastExitCode, args)) break;
                rc = executeBlock(body, args, depth+1);
                lastExitCode = rc;
            }
            continue;
        }

        // for loop: for VAR in list; do ... done
        if (starts_with(raw, "for ")){
            std::string header = raw;
            std::vector<std::string> body;
            bool haveDo = header.find(" do") != std::string::npos || ends_with(header, ";do") || ends_with(header, "; do");
            if (haveDo){ size_t p=header.find("do"); if(p!=std::string::npos){ std::string after=trim(header.substr(p+2)); if(!after.empty()) body.push_back(after); header=trim(header.substr(0,p)); } }
            else {
                while (idx + 1 < lines.size()){
                    std::string l = trim(lines[idx+1]);
                    if (l == "do") { idx += 1; haveDo = true; break; }
                    idx += 1;
                }
            }
            while (idx + 1 < lines.size()){
                std::string l = lines[idx+1];
                if (trim(l) == "done") { idx += 1; break; }
                body.push_back(l);
                idx += 1;
            }
            // parse header: for VAR in LIST
            std::string hdr = trim(header.substr(4));
            auto parts = split_words(expandLine(hdr, lastExitCode, args));
            if (parts.size() >= 3 && parts[1]=="in"){
                std::string varName = parts[0];
                std::vector<std::string> list(parts.begin()+2, parts.end());
                int rc = 0;
                for (const auto& val : list){ variables[varName] = val; rc = executeBlock(body, args, depth+1); lastExitCode = rc; }
            }
            continue;
        }

        // function call?
        {
            auto words = split_words(line);
            if (!words.empty()){
                auto itf = functions.find(words[0]);
                if (itf != functions.end()){
                    std::vector<std::string> fargs;
                    if (words.size()>1) fargs.assign(words.begin()+1, words.end());
                    // save and set positional args
                    int rc = executeBlock(itf->second.body, fargs, depth+1);
                    lastExitCode = rc;
                    continue;
                }
            }
        }

        // normal command
        try {
            lastExitCode = shell->processCommand(line);
        } catch (const std::exception& e) {
            std::cerr << RED << "Script error: " << e.what() << RESET << std::endl;
            lastExitCode = 1;
        }
    }
    return lastExitCode;
}

int ScriptInterpreter::executeLines(std::istringstream& stream,
                                    std::vector<std::string> args,
                                    int depth) {
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(stream, line)){
        // skip shebang
        if (lines.empty() && starts_with(line, "#!")) continue;
        lines.push_back(line);
    }
    return executeBlock(lines, std::move(args), depth);
}

}
