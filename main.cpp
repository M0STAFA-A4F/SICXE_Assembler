#include <bits/stdc++.h>

using namespace std;

unordered_set <string> directive = {
    "BYTE","WORD","RESB","RESW","RESDW","EQU"
};

unordered_map <string,int> regTrans = {
    {"A",0x00},
    {"X",0x01},
    {"L",0x02},
    {"B",0x03},
    {"S",0x04},
    {"T",0x05},
    {"F",0x06},
    {"PC",0x08},
    {"SW",0x09}
};

unordered_map <string,int> format1 = {
    {"FIX",0xC4},
    {"FLOAT",0xC0},
    {"HIO",0xF4},
    {"NORM",0xC8},
    {"SIO",0xF0},
    {"TIO",0xF8}
};

unordered_map <string,int> format2 = {
    {"ADDR",0x90},
    {"CLEAR",0xB4},
    {"COMPR",0xA0},
    {"DIVR",0x9C},
    {"MULR",0x98},
    {"RMO",0xAC},
    {"SHIFTL",0xA4},
    {"SHIFTR",0xA8},
    {"SUBR",0x94},
    {"SVC",0xB0},
    {"TIXR",0xB8}
};

unordered_map <string,int> format345 = {
    {"ADD",0x18},
    {"ADDF",0x58},
    {"AND",0x40},
    {"COMP",0x28},
    {"COMPF",0x88},
    {"DIV",0x24},
    {"DIVF",0x64},
    {"J",0x3C},
    {"JEQ",0x30},
    {"JGT",0x34},
    {"JLT",0x38},
    {"JSUB",0x48},
    {"LDA",0x00},
    {"LDB",0x68},
    {"LDCH",0x50},
    {"LDF",0x70},
    {"LDL",0x08},
    {"LDS",0x6C},
    {"LDT",0x74},
    {"LDX",0x04},
    {"LPS",0xD0},
    {"MUL",0x20},
    {"MULF",0x60},
    {"OR",0x44},
    {"RD",0xD8},
    {"RSUB",0x4C},
    {"TD",0xE0},
    {"TIX",0x2C},
    {"WD",0xDC},
    {"SSK",0xEC},
    {"STA",0x0C},
    {"STB",0x78},
    {"STCH",0x54},
    {"STF",0x80},
    {"STI",0xD4},
    {"STL",0x14},
    {"STSW",0xE8},
    {"STT",0x84},
    {"STX",0x10},
    {"SUB",0x1C},
    {"SUBF",0x5C}
};

unordered_map <string,pair<char,int>> symTab;

unordered_map <string,pair<string,int>> litTab;

vector <int> addresses;

vector <string> objCode;

/********************************************** general functions **********************************************/

void spliting(char *line, char *symbol, vector<string> &result)
{
    char *token = strtok(line, symbol);
    while (token != NULL)
    {
        result.push_back(token);
        token = strtok(NULL, symbol);
    }
}

void split(char *line,vector<string> &result)
{
    string str;
    stringstream ss;

    ss<<line;

    for(int i=0; ss>>str && str[0]!='.' && i<3; i++)
        result.push_back(str);

    ss.clear();
}

void negFisrtChar(string &str)
{
    stringstream ss;

    int n = str.size();
    for(int i=1; i<n; i++)
        ss<<str[i];
    ss>>str;

    ss.clear();
}

void toUpper(char *line)
{
    for(int i=0; line[i]!='\0'; i++)
        line[i] = toupper(line[i]);
}

string toString(int number, int formatNo)
{
    stringstream ss;
    string str;
    char s[15];
    int len;

    switch(formatNo)
    {
        case 1:sprintf(s,"%02x",number);break;
        case 2:sprintf(s,"%04x",number);break;
        case 3:
        case 5:sprintf(s,"%06x",number);break;
        case 4:sprintf(s,"%08x",number);break;
    }

    len = strlen(s);
    for(int i=0; i<len; i++)
        ss<<s[i];

    ss>>str;

    ss.clear();
    return str;
}

int toInt(string str)
{
    stringstream ss;
    int number;

    ss<<str;
    ss>>number;

    ss.clear();
    return number;
}

int toHex(string str)
{
    stringstream ss;
    int number;

    ss<<str;
    ss>>hex>>number;

    ss.clear();
    return number;
}

int isFormat1(string str)
{
    return format1.find(str) != format1.end();
}

int isFormat2(string str)
{
    return (format2.find(str) != format2.end())*2;
}

int isFormat345(string str)
{
    return (format345.find(str) != format345.end())*3;
}

bool isMnemonic(string str, int &format_no)
{
    format_no = 0;

    if(str[0] == '+')
    {
        format_no++;
        negFisrtChar(str);
    }
    else if(str[0] == '$')
    {
        format_no += 2;
        negFisrtChar(str);
    }

    if(isFormat1(str) + isFormat2(str) + isFormat345(str))
        return format_no += isFormat1(str) + isFormat2(str) + isFormat345(str);
    else
        return 0;
}

bool isDirective(string str)
{
    return directive.find(str) != directive.end();
}

bool isRegister(string str)
{
    return regTrans.find(str) != regTrans.end();
}

bool inSymbolTable(string str)
{
    return symTab.find(str) != symTab.end();
}

bool inLiteralTable(string str)
{
    return litTab.find(str) != litTab.end();
}

void errorMessage(char *instruction, int line_no)
{
    printf("error in line %d --> ",line_no);
    puts(instruction);
    exit(0);
}

int getNumber(string str)
{
    string tmp;
    int n = str.size();

    for(int i=2; i<n-1; i++)
        tmp[i-2] = str[i];

    return toHex(tmp);
}

string getValue(string number)
{
    stringstream ss;
    string rec;
    if(number[number[0]=='='] == 'X')
    {
        for(unsigned int i=2+(number[0]=='='); i<number.size()-1; i++)
            ss<<number[i];
        ss>>rec;
    }
    else if(number[number[0]=='='] == 'C')
    {
        for(unsigned int i=2+(number[0]=='='); i<number.size()-1; i++)
            ss<<setprecision(2)<<fixed<<hex<<(int)number[i];
        ss>>rec;
    }
    ss.clear();
    return rec;
}

int mathEq(string equation)
{
    stringstream ss;
    string parametars[3];
    char oprator;
    int res;
    int n = equation.size();

    for(int i=0; i<n; i++)
    {
        if(equation[i] == '-')
        {
            ss<<"\n";
            oprator = '-';
        }
        else
        {
            ss<<equation[i];
        }
    }

    int i=0;
    while(ss>>parametars[i++]);
    ss.clear();

    switch(oprator)
    {
        case '-': res = symTab[parametars[0]].second - symTab[parametars[1]].second;break;
    }

    return res;
}

/********************************************** PASS 1 **********************************************/

void incAddress(int &address, int format_no)
{
    switch(format_no)
    {
        case 1: address += 1;break;
        case 2: address += 2;break;
        case 3: address += 3;break;
        case 4: address += 4;break;
        case 5: address += 3;break;
    }
}

void insInSymTable(FILE *sym_table, string symbol, char type, int address)
{
    fprintf(sym_table,"%s\t%c\t%04x\n",&symbol[0],type,address);
    symTab[symbol].first = type;
    symTab[symbol].second = address;
}

void insInLitTable(FILE *lit_table, string literal, string value, int address)
{
    fprintf(lit_table,"%s\t%s\t%04x\n",&literal[0],&value[0],address);
    litTab[literal].first = value;
    litTab[literal].second = address;
}

void pass1(FILE *input, FILE *sym_table, FILE *lit_table)
{
    char instruction[100];
    int line_no = 0;
    int cur_address;
    int format_no;

    if(fgets(instruction,100,input))
    {
        line_no++;

        vector <string> code;
        split(instruction,code);

        if(code.size() < 3 || code[0].size() > 6 || code[1] != "START")
        {
            errorMessage(instruction,line_no);
        }
        else
        {
            cur_address = toHex(code[2]);
            insInSymTable(sym_table,code[0],'R',cur_address);
        }
    }

    while(fgets(instruction,100,input))
    {
        line_no++;

        vector <string> code;
        split(instruction,code);

        if(code[0] == "BASE" || code[0] == "LTORG" || code[0] == "END")
            continue;

        addresses.push_back(cur_address);

        if(code.size() == 3)
        {
            if(!inSymbolTable(code[0]) && isMnemonic(code[1],format_no))
            {
                insInSymTable(sym_table,code[0],'R',cur_address);
                incAddress(cur_address,format_no);
            }
            else if(!inSymbolTable(code[0]) && isDirective(code[1]))
            {
                if(code[1] == "BYTE")
                {
                    insInSymTable(sym_table,code[0],'R',cur_address);
                    if(code[2][0] == 'X' && code[2][1] == 39 && code[2][code[2].size()-1] == 39)
                        cur_address += (code[2].size()-3)/2;
                    else if(code[2][0] == 'C' && code[2][1] == 39 && code[2][code[2].size()-1] == 39)
                        cur_address += code[2].size()-3;
                    else
                        errorMessage(instruction,line_no);
                }
                else if(code[1] == "WORD")
                {
                    insInSymTable(sym_table,code[0],'R',cur_address);
                    cur_address += 3;
                }
                else if(code[1] == "RESB")
                {
                    insInSymTable(sym_table,code[0],'R',cur_address);
                    if(code[2][0] == 'X' && code[2][1] == 39 && code[2][code[2].size()-1] == 39)
                        cur_address += getNumber(code[2]);
                    else
                        cur_address += toInt(code[2]);
                }
                else if(code[1] == "RESW")
                {
                    insInSymTable(sym_table,code[0],'R',cur_address);
                    if(code[2][0] == 'X' && code[2][1] == 39 && code[2][code[2].size()-1] == 39)
                        cur_address += 3*getNumber(code[2]);
                    else
                        cur_address += 3*toInt(code[2]);
                }
                else if(code[1] == "RESDW")
                {
                    insInSymTable(sym_table,code[0],'R',cur_address);
                    if(code[2][0] == 'X' && code[2][1] == 39 && code[2][code[2].size()-1] == 39)
                        cur_address += 6*getNumber(code[2]);
                    else
                        cur_address += 6*toInt(code[2]);
                }
                else if(code[1] == "EQU")
                {
                    if(code[2] == "*")
                        insInSymTable(sym_table,code[0],'R',cur_address);
                    else
                    {
                        insInSymTable(sym_table,code[0],'A',mathEq(code[2]));
                    }
                }
            }
            else
            {
                errorMessage(instruction,line_no);
            }
        }
        else if(code.size() == 2)
        {
            if(isMnemonic(code[0],format_no))
            {
                incAddress(cur_address,format_no);
            }
            else if(!inSymbolTable(code[0]) && isMnemonic(code[1],format_no))
            {
                insInSymTable(sym_table,code[0],'R',cur_address);
                incAddress(cur_address,format_no);
            }
            else if(code[0] == "*")
            {
                if(code[1][0] == '=' && (code[1][1] == 'X' || code[1][1] == 'C') && code[1][2] == 39 && code[1][code[1].size()-1] == 39)
                {
                    string value = getValue(code[1]);
                    insInLitTable(lit_table,code[1],value,cur_address);
                    cur_address += value.size()/2;
                }
                else
                    errorMessage(instruction,line_no);

            }
            else
            {
                errorMessage(instruction,line_no);
            }
        }
        else if(code.size() == 1)
        {
            if(isMnemonic(code[0],format_no))
            {
                incAddress(cur_address,format_no);
            }
            else
            {
                errorMessage(instruction,line_no);
            }
        }
        else
        {
            errorMessage(instruction,line_no);
        }
    }
}

/********************************************** PASS 2 **********************************************/

bool inPcRange(string label, int nextAddress)
{
    if(inSymbolTable(label))
        return symTab[label].second-nextAddress <= 2047 && symTab[label].second-nextAddress >= -2048;
    else if(inLiteralTable(label))
        return litTab[label].second-nextAddress <= 2047 && litTab[label].second-nextAddress >= -2048;
    return 0;
}

bool inBaseRange(string label, int baseAddress)
{
    if(inSymbolTable(label))
        return symTab[label].second-baseAddress <= 4095 && symTab[label].second-baseAddress >= 0;
    else if(inLiteralTable(label))
        return litTab[label].second-baseAddress <= 4095 && litTab[label].second-baseAddress >= 0;
    return 0;
}

string getObjectCode(string men, int formatNo, char mode, string label1, string label2, int baseAddress, int nextAddress)
{
    int objectCode;

    if(formatNo == 1)
    {
        objectCode = format1[men];
    }
    else if(formatNo == 2)
    {
        objectCode = format2[men] << 8;

        objectCode |= (regTrans[label1] << 4);
        objectCode |= (regTrans[label2] != regTrans[label1])? regTrans[label2]: 0;
    }
    else
    {
        objectCode = format345[men] << 4;

        objectCode |= (((mode == '@' || mode == 0) && formatNo != 5) << 5);
        objectCode |= (((mode == '#' || mode == 0) && formatNo != 5) << 4);
        objectCode |= ((label2 == "X") << 3);
        objectCode |= ((inBaseRange(label1,baseAddress) && !inPcRange(label1,nextAddress) && formatNo!=4) << 2);
        objectCode |= ((inPcRange(label1,nextAddress) && formatNo!=4) << 1);
        objectCode |= (formatNo == 4);

        if(!inSymbolTable(label1) && !inLiteralTable(label1) && mode == '#')
        {
            objectCode <<= (formatNo == 4)? 20: 12;
            objectCode |= toInt(label1);
        }
        else if(formatNo == 4)
        {
            objectCode <<= 20;
            objectCode |= (inSymbolTable(label1))? symTab[label1].second : litTab[label1].second;
        }
        else if(inPcRange(label1,nextAddress))
        {
            int disp;

            objectCode <<= 12;
            objectCode |= (inSymbolTable(label1))? (disp = symTab[label1].second-nextAddress)&0xfff : (disp = litTab[label1].second-nextAddress)&0xfff;

            if(formatNo == 5)
            {
                objectCode |= ((disp%2 == 0) << 17);
                objectCode |= ((disp > 0) << 16);
            }
        }
        else if(inBaseRange(label1,baseAddress))
        {
            int disp;

            objectCode <<= 12;
            objectCode |= (inSymbolTable(label1))? disp = symTab[label1].second-baseAddress : disp = litTab[label1].second-baseAddress;

            if(formatNo == 5)
            {
                objectCode |= ((disp%2 == 0) << 17);
                objectCode |= ((disp > 0) << 16);
            }
        }
        else
        {
            cout<<"out of range\n";
        }
    }

    return toString(objectCode,formatNo);
}

void objectCodes(FILE *input, char *programName)
{
    char instruction[100];
    int line_no = 0;
    int format_no;
    int baseAddress = 0;
    int nextAddNo = 0;

    if(fgets(instruction,100,input))
    {
        line_no++;

        vector <string> code;
        split(instruction,code);

        sprintf(programName,"%s",&code[0][0]);
    }

    while(fgets(instruction,100,input))
    {
        line_no++;

        vector <string> code;
        split(instruction,code);

        if(code[0] == "LTORG" || code[0] == "END")
        {
            continue;
        }
        else if(code[0] == "BASE")
        {
            baseAddress = symTab[code[1]].second;
            continue;
        }

        nextAddNo++;

        if(code.size() == 3)
        {
            vector <string> label;
            spliting(&code[2][0],",",label);

            char mode = 0;
            string label1 = label[0];
            string label2 = label[label.size()-1];

            if(label1[0] == '@' || label1[0] == '#')
            {
                mode = label1[0];
                negFisrtChar(label1);
            }

            if(isMnemonic(code[1],format_no) && (inSymbolTable(label1) || inLiteralTable(label1) || mode == '#' || (isRegister(label1) && isRegister(label2))))
            {
                string men = code[1];
                if(format_no == 4 || format_no == 5)
                    negFisrtChar(men);

                objCode.push_back(getObjectCode(men, format_no, mode, label1, label2, baseAddress, addresses[nextAddNo]));
            }
            else if(isDirective(code[1]))
            {
                if(code[1] == "BYTE")
                {
                    objCode.push_back(getValue(code[2]));
                }
                else if(code[1] == "WORD")
                {
                    objCode.push_back(toString(toInt(code[2]),3));
                }
                else
                {
                    objCode.push_back("------");
                }
            }
            else
            {
                errorMessage(instruction,line_no);
            }
        }
        else if(code.size() == 2)
        {
            vector <string> label;
            spliting(&code[1][0],",",label);

            char mode = 0;
            string label1 = label[0];
            string label2 = label[label.size()-1];

            if(label1[0] == '@' || label1[0] == '#')
            {
                mode = label1[0];
                negFisrtChar(label1);
            }

            if(isMnemonic(code[0],format_no) && (inSymbolTable(label1) || inLiteralTable(label1) || mode == '#' || (isRegister(label1) && isRegister(label2))))
            {
                string men = code[0];
                if(format_no == 4 || format_no == 5)
                    negFisrtChar(men);

                objCode.push_back(getObjectCode(men, format_no, mode, label1, label2, baseAddress, addresses[nextAddNo]));
            }
            else if(isMnemonic(code[1],format_no))
            {
                if(format_no == 1)
                    objCode.push_back(toString(format1[code[1]],format_no));
                else if(format_no == 4)
                    objCode.push_back("4f100000");
                else
                    objCode.push_back("4f0000");

            }
            else if(code[0] == "*")
            {
                objCode.push_back(getValue(code[1]));
            }
            else
            {
                errorMessage(instruction,line_no);
            }
        }
        else if(code.size() == 1)
        {
            if(isMnemonic(code[0],format_no))
            {
                if(format_no == 1)
                    objCode.push_back(toString(format1[code[0]],format_no));
                else if(format_no == 4)
                    objCode.push_back("4f100000");
                else
                    objCode.push_back("4f0000");
            }
        }
    }
}

void Mrecord(FILE *input, FILE *output)
{
    char instruction[100];
    int formatNO = 0;

    fgets(instruction,100,input);

    for(int i=0; fgets(instruction,100,input); i++)
    {
        vector <string> code;
        split(instruction,code);

        if(code[0] == "BASE" || code[0] == "LTORG" || code[0] == "END")
        {
            i--;
            continue;
        }

        if(isMnemonic(code[code.size()-2],formatNO))
        {
            if(formatNO == 4)
            {
                vector <string> label;
                spliting(&code[code.size()-1][0],",",label);

                char mode = 0;
                string label1 = label[0];

                if(label1[0] == '#')
                {
                    mode = label1[0];
                    negFisrtChar(label1);
                }

                if(inSymbolTable(label1))
                {
                    fprintf(output,"M.%06x.05\n",addresses[i]+1);
                }
            }
        }
    }
}

void HTMErecord(FILE *input, FILE *output, char *progname, int start_address, int last_address)
{
    fprintf(output,"H.%-6s.%06x.%06x\n",progname,start_address,last_address-start_address);

    for(int i=0; i<objCode.size(); i++)
    {
        stringstream ss;

        if(objCode[i] == "------")
            continue;

        int cur_address = addresses[i];
        int cnt = 0;

        for( ; objCode[i] != "------" && i<objCode.size(); i++)
        {
            cnt += objCode[i].size();

            if(cnt > 60)
                break;

            ss<<objCode[i]<<endl;
        }

        i--;

        fprintf(output,"T.%06x.%02x",cur_address,addresses[i]+(objCode[i].size()/2)-cur_address);

        char rec[8];
        while(ss>>rec)
            fprintf(output,".%s",rec);
        fprintf(output,"\n");

        ss.clear();
    }

    Mrecord(input, output);

    fprintf(output,"E.%06x",start_address);
}

void pass2(FILE *input, FILE *object_code, FILE *output)
{
    char programName[10];

    objectCodes(input, programName);

    for(auto obj:objCode)
        fprintf(object_code,"%s\n",&obj[0]);

    fclose(input);
    input = fopen("code.txt","r");

    HTMErecord(input, output, programName, addresses[0], addresses[addresses.size()-1]+objCode[objCode.size()-1].size()/2);
}

/********************************************** file function **********************************************/

void reWriteFile(FILE *input, FILE *output)
{
    char instruction[100];
    stringstream ss;
    string str;
    set <string> lit;

    while(fgets(instruction,100,input))
    {
        toUpper(instruction);

        if(instruction[0] == '.' || instruction[0] == '\n')
            continue;

        ss<<instruction;
        while(ss>>str)
        {
            if(str[0] == '=')
                lit.insert(str);

            if(str == "LTORG" || str == "END")
            {
                if(str == "END")
                {
                    fprintf(output,"%s ",&str[0]);
                    ss>>str;
                    fprintf(output,"%s\n",&str[0]);
                }
                else
                {
                    fprintf(output,"%s\n",&str[0]);
                }
                for(auto i=lit.begin(); i!=lit.end(); i++)
                {
                    str = *i;
                    fprintf(output,"* %s%c",&str[0],"\n "[lit.size() == 1]);
                }
                lit.clear();
            }
            else
            {
                fprintf(output,"%s ",&str[0]);
            }
        }
        fprintf(output,"\n");
        ss.clear();
    }
}

/********************************************** main **********************************************/

int main()
{
    FILE *input, *output, *sym_table, *lit_table, *object_code;

    input = fopen("input.txt","r");
    output = fopen("code.txt","w");

    reWriteFile(input,output);

    fclose(input);
    fclose(output);

    input = fopen("code.txt","r");
    sym_table = fopen("symbol_table.txt","w");
    lit_table = fopen("literal_table.txt","w");

    pass1(input, sym_table, lit_table);

    fclose(input);
    fclose(sym_table);
    fclose(lit_table);

    input = fopen("code.txt","r");
    object_code = fopen("object_code.txt","w");
    output = fopen("HTME_Records.txt","w");

    pass2(input, object_code, output);

    fclose(input);
    fclose(object_code);
    fclose(output);

    return 0;
}
