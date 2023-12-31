#include <unordered_map>
#include <stdio.h>
#include <string>
#include <vector>

using std::unordered_map;
using std::string;
using std::vector;

const int TAPE_LEN = 30*1000;
struct Jumps {
    // forward maps from a forward brace index
    // to the matching closing brace.
    unordered_map<int, int> forward;
    // backward maps from closing brace index
    // to the matching opening brace
    unordered_map<int, int> backward;
};
    
Jumps build_jumps(const string& prog) {
    /*
    build tables to lookup the matching
    brace for a program.
    */
    Jumps result;
    vector<int> stack;
    int i = 0;
    while(i < prog.size()) {
        if (prog[i] == '[') {
            stack.push_back(i);
        } else if (prog[i] == ']') {
            if (stack.size() == 0) {
                puts("no [ on stack.");
                exit(1);
            }
            int match = stack.back(); stack.pop_back();
            result.backward[i] = match;
            result.forward[match] = i;
        }
        i += 1;
    }
    if (stack.size() > 0) {
        puts("jump stack not empty");
        exit(1);
    }
    return result;
}

int jump(const unordered_map<int, int>& jumps,
    int index) {
    auto it = jumps.find(index);
    if (it != jumps.end()) {
        return it->second;
    } else {
        printf("no jumps for %d\n", index);
        exit(1);
    }
}
void bf(const string& prog) {
    vector<uint8_t> tape;
    tape.resize(TAPE_LEN);
    Jumps jumps = build_jumps(prog);
    int ptr = 0;
    int pc = 0;
    int len = prog.length();
    while(pc < len && ptr >= 0 && ptr < tape.size()) {
        // printf("pc: %d\n", pc);
        char c = prog[pc];
        switch(c) {
        case '+':
            tape[ptr]++;
            break;
        case '-':
            tape[ptr]--;
            break;
        case '>':
            ptr += 1;
            break;
        case '<':
            ptr -= 1;
            break;
        case '[':
            if(tape[ptr] == 0)
                pc = jump(jumps.forward, pc);
            break;
        case ']':
            if(tape[ptr] != 0)
                pc = jump(jumps.backward, pc);
            break;
        case '.':
            printf("%c", (char)tape[ptr]);
            // fflush(stdout);  // flush for debugging
            break;
        case ',':
            puts(", unimplemented");
            exit(1);
            break;
        default:
            printf("unhandled instn: '%c'\n", c);
            exit(1);
        }
        pc += 1;
    }
}

string slurp(const string& path) {
    string output;
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        perror("failed to open file");
        exit(1);
    }
    char buffer[1024];
    int goal = sizeof(buffer)-1;
    while(true) {
        size_t n = fread(buffer, 1, goal, f);
        if (n < 0) {
            perror("file read error");
            exit(1);
        } 
        buffer[n] = 0;
        output.append(buffer, n);
        if (n < goal)
            break;
    }
    return output;
}

string clean(const string& input) {
    string program;
    program.reserve(input.length());
    int i = 0;
    while (i < input.length()) {
        char c = input[i];
        if (c == ';') {
            // drop until newline
            while (i < input.length() && 
                input[i] != '\n') {
                i++;
            }
            continue;
        }

        switch(c) {
            case '\n':
                [[fallthrough]];
            case ' ':
                break;
            default:
                program.push_back(c);
        }
        i++;
    }
    return program;
}
    
int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: ./bf file.bf");
        return 0;
    }
    string program = slurp(argv[1]);
    program = clean(program);
    bf(program);
    return 0;
}
