#include<regex>
#include <sstream>
#include<cctype>
#include<algorithm>
#include<vector>
#include<iostream>
#include<fstream>
#include<map>
#include<stack>
#include<queue>
#include<string>


using namespace std;

map<string, int> StrToBCode = {
{"#line", 10},
{"#id", 11},
{"#const", 12},
{"#if", 13},
{"#goto", 14},
{"#print", 15},
{"#stop", 16},
{"#op", 17}
};


vector<vector<string>> rules = {
{},
{"pgm", "pgm", "line"},
{"pgm", "EOF"},
{"line", "stmt", "line_num"},
{"stmt", "asgmnt"},
{"stmt", "if"},
{"stmt", "print"},
{"stmt", "goto"},
{"stmt", "stop"},
{"asgmnt", "exp", "=", "id"},
{"exp", "exp'", "term"},
{"exp'", "term", "+"},
{"exp'", "term", "-"},
{"exp'"},
{"term", "id"},
{"term", "const"},
{"if", "line_num", "cond", "IF"},
{"cond", "cond'", "term"},
{"cond'", "term", "<"},
{"cond'", "term", "="},
{"print", "id", "PRINT"},
{"goto", "line_num", "GOTO"},
{"stop", "STOP"}
};

stack<string> parsingStack;
queue<pair<int,int>> outputQueue;
bool alternativeGoto=0;

map<string,int> nonTerminalMap = {
{"pgm", 0},
{"line", 1},
{"stmt", 2},
{"asgmnt", 3},
{"exp", 4},
{"exp'", 5},
{"term", 6},
{"if", 7},
{"cond", 8},
{"cond'", 9},
{"print", 10},
{"goto", 11},
{"stop", 12}
};
map<string, int> terminalMap = {
{"line_num", 0},
{"id", 1},
{"const", 2},
{"IF", 3},
{"PRINT", 4},
{"GOTO", 5},
{"STOP", 6},
{"+", 7},
{"-", 8},
{"<", 9},
{"=", 10},
{"EOF", 11}
};

int parsingTable[13][12] = {
{1,0,0,0,0,0,0,0,0,0,0,2},
{3,0,0,0,0,0,0,0,0,0,0,0},
{0,4,0,5,6,7,8,0,0,0,0,0},
{0,9,0,0,0,0,0,0,0,0,0,0},
{0,10,10,0,0,0,0,0,0,0,0,0},
{13,0,0,0,0,0,0,11,12,0,0,13},
{0,14,15,0,0,0,0,0,0,0,0,0},
{0,0,0,16,0,0,0,0,0,0,0,0},
{0,17,17,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,18,19,0},
{0,0,0,0,20,0,0,0,0,0,0,0},
{0,0,0,0,0,21,0,0,0,0,0,0},
{0,0,0,0,0,0,22,0,0,0,0,0}
};

string getTerminalType(string token);
void sendToOutput(string terminal, int token);
int getId (char c);

class writer {
ofstream output;
public:
writer() {
}

writer(char* fname) {
output.open(fname, ofstream::out);
}
~writer() {
output <<"0";
output.close();
}
void open(char* fname) {
//cout <<"run this\n";
output.open(fname);
}

void append(pair<int,int> p) {
output <<p.first <<" " <<p.second <<" ";
}
};

class scanner {
ifstream input;
public:
string token="";
stringstream ss;
bool eof=0;
scanner() {
}

scanner(char* fname) {
input.open(fname);
getNext();
}
~scanner() {
input.close();
}
void open(char* fname) {
//cout <<"run this\n";
input.open(fname);
string temp ((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
temp = regex_replace(temp, regex("\\s+$"), "");
ss <<temp;
getNext();
}

void getNext() {
//cout <<"run that\n";
//cout <<"next is" <<input.peek() <<endl;
if (ss.eof()) {
cout <<"end reach\n";
eof=1;
}  else {
cout <<"end not reach\n";
ss >>token;
cout <<"recieve " <<token <<endl;
}
}
};

scanner sc;
writer wr;

void applyRule (string token) {
string terminal = getTerminalType(token);
string nonTerminal = parsingStack.top();
cout <<"terminal:" <<terminal <<", non terminal:" <<nonTerminal <<endl;
int follow=0;
if (terminal == "number") {
follow=parsingTable[nonTerminalMap[nonTerminal]][0];
if (follow==0) follow=parsingTable[nonTerminalMap[nonTerminal]][2];
} else {
follow=parsingTable[nonTerminalMap[nonTerminal]][terminalMap[terminal]];
}
cout <<"follow:" <<follow <<endl;
if (follow == 0) {
cout <<"Syntax error, please check your source code." <<endl;
exit(1);
}
vector<string> rule = rules[follow];
if (parsingStack.top() == rule[0]) {
parsingStack.pop();
} else {
cout <<"Syntax error, please check your source code." <<endl;
exit(1);
}
for (int i=1; i<rule.size(); i++) {
parsingStack.push(rule[i]);
}
}

void sendToOutput(string terminal, int token) {
cout <<"sending (" <<terminal <<"," <<token <<") to output\n";
pair<int, int> buf;
buf.first = StrToBCode[terminal];
buf.second = token;
wr.append(buf);
}

bool matchToken (string token, scanner &sc) {
string terminal = getTerminalType(token);
string top = parsingStack.top();
cout <<"trying to match " <<terminal <<" and " <<top <<endl;
bool result=0;
if ((terminal == "number") && (top == "const")) {
cout <<"case 1 match\n";
result=1;
sendToOutput("#const", stoi(token));
} else if ((terminal == "number") && (top == "line_num")) {
cout <<"case 2 match\n";
result=1;
if (alternativeGoto==1) {
sendToOutput("#goto", stoi(token));
alternativeGoto=0;
cout <<"alternative mode deactivated\n";
} else {
sendToOutput("#line", stoi(token));
}
} else if (terminal == top) {
cout <<"case 3 match\n";
result=1;
if (terminal=="id") {
sendToOutput("#id", getId(token[0]));
} else if (terminal=="const") {
sendToOutput("#const", stoi(token));
} else if (terminal=="IF") {
alternativeGoto=1;
cout <<"alternative mode activated\n";
sendToOutput("#if", 0);
} else if (terminal=="GOTO") {
alternativeGoto=1;
cout <<"alternative mode activated\n";
} else if (terminal=="PRINT") {
sendToOutput("#print", 0);
} else if (terminal=="STOP") {
sendToOutput("#stop", 0);
} else if (terminal=="+") {
sendToOutput("#op", 1);
} else if (terminal=="-") {
sendToOutput("#op", 2);
} else if (terminal=="<") {
sendToOutput("#op", 3);
} else if (terminal=="=") {
sendToOutput("#op", 4);
}
}
if (result==1) {
cout <<"match:" <<token <<" and " <<top <<" success" <<endl;
parsingStack.pop();
sc.getNext();
} else {
cout <<"match fail" <<endl;
}
return result;
}

int getId (char c) {
if (c > 96) {
return c-96;
} else {
return c-64;
}
}

bool isAlpha(string token) {
bool result = 0;
if (token.length() == 1) {
if (isalpha(token[0]) != 0) {
result = 1;
}
}
}

bool isNumber(string token) {
bool result = 1;
for(int i=0; i< token.length(); i++) {
if (isdigit(token[i]) == 0) {
result = 0;
break;
}
}
return result;
}

string getTerminalType(string token) {
transform(token.begin(), token.end(), token.begin(), ::tolower);
if (token == "if") {
return "IF";
} else if (token == "print") {
return "PRINT";
} else if (token == "goto") {
return "GOTO";
} else if (token == "stop") {
return "STOP";
} else if (token == "+") {
return "+";
} else if (token == "-") {
return "-";
} else if (token == "<") {
return "<";
} else if (token == "=") {
return "=";
} else if (isNumber(token)) {
return "number";
} else if (isAlpha(token)) {
return "id";
}
}

int main (int argc, char* argv[]) {
if (argc == 3) {
sc.open(argv[1]);
wr.open(argv[2]);
} else {
cout <<"invalid parameter" <<endl;
return 1;
}
parsingStack.push("EOF");
parsingStack.push("pgm");
cout <<sc.token <<"," <<sc.eof <<endl;
while (sc.eof == 0) {
cout <<sc.token <<endl;
while (matchToken(sc.token, sc) == 0) {
cout <<"match fail handler\n";
applyRule(sc.token);
}
}
return 0;
}