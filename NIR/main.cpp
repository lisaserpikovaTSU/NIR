
#include <iostream>
#include "json.hpp"
#include <fstream>
#include <string>
#include <iomanip>

using json = nlohmann::json;

class BoolVector {
private:
    unsigned char *v;
    int nbit;
    int m;
    
    void resizeA(int);
    
public:
    BoolVector(int nn = 1);
    BoolVector(const char*);
    BoolVector(const BoolVector &);
    ~BoolVector();
    
    void Set1(int);
    void Set0(int);
    int operator[](int);
    int operator[](int)const;
    BoolVector operator = (const BoolVector &);
    
    bool operator == (const BoolVector &);
    
    BoolVector operator | (const BoolVector &) const;
    BoolVector operator & (const BoolVector &) const;
    BoolVector operator ~() const;
    
    friend std::ostream& operator << (std::ostream&, const BoolVector &);
    friend std::istream& operator >> (std::istream&, BoolVector &);
    
    friend class CNF;
};

BoolVector::BoolVector(int nn){
    nbit = nn;
    m = (nn + 7) / 8;
    v = new unsigned char[m];
    for(int i = 0; i < m; i++){
        v[i] = 0;
    }
}

BoolVector::BoolVector(const char* str){
    nbit = strlen(str);
    m = (nbit + 7) / 8;
    v = new unsigned char[m];
    for(int i=0; i<m; i++){
        v[i] = 0;
    }
    for(int i=0; i<nbit; i++){
        if(str[i] == '1'){
            Set1(i);
        }
    }
}

BoolVector::BoolVector(const BoolVector &other){
    nbit = other.nbit;
    m = other.m;
    v = new unsigned char[m]();
    for(int i=0; i<m; i++){
        v[i] = other.v[i];
    }
}

BoolVector::~BoolVector(){
    delete [] v;
}

void BoolVector::Set1(int index){
    if (index >= nbit) resizeA(nbit+1);
    v[index/8] |= (1 << (index%8));
}

void BoolVector::Set0(int index){
    if (index >= nbit) resizeA(m+1);
    v[index/8] &= ~(1 << (index%8));
}

int BoolVector::operator[](int index){
    return (v[index/8] >> (index%8)) & 1;
}

int BoolVector::operator[](int index) const{
    return (v[index/8] >> (index%8)) & 1;
}

void BoolVector::resizeA(int Nnbit) {
    int Nm = (Nnbit + 7) / 8;
    unsigned char *Nv = new unsigned char[Nm]();
    
    for(int i = 0; i < m; i++) {
        Nv[i] = v[i];
    }
    
    delete [] v;
    v = Nv;
    nbit = Nnbit;
    m = Nm;
}

BoolVector BoolVector::operator=(const BoolVector &other){
    if(this == &other) return *this;
    delete [] v;
    nbit = other.nbit;
    m = other.m;
    v = new unsigned char[m]();
    for(int i=0; i<m; i++){
        v[i] = other.v[i];
    }
    return *this;
}

bool BoolVector::operator==(const BoolVector &other){
    if(nbit != other.nbit) return false;
    for(int i=0; i<m; i++){
        if(v[i] != other.v[i]) return false;
    }
    return true;
}

BoolVector BoolVector::operator|(const BoolVector &other) const{
    BoolVector res(std::max(nbit, other.nbit));
    int minM = std::min(m, other.m);
    for(int i=0; i<minM; i++){
        res.v[i] = v[i] | other.v[i];
    }
    if(m > other.m) {
        for(int i = minM; i < m; i++) {
            res.v[i] = v[i];
        }
    } else {
        for(int i = minM; i < other.m; i++) {
            res.v[i] = other.v[i];
        }
    }
    
    return res;
}

BoolVector BoolVector::operator&(const BoolVector &other) const{
    BoolVector res(std::max(nbit, other.nbit));
    int minM = std::min(m, other.m);
    for(int i=0; i<minM; i++){
        res.v[i] = v[i] & other.v[i];
    }
    return res;
}

BoolVector BoolVector::operator~() const{
    BoolVector res(nbit);
    for(int i=0; i<m; i++){
        res.v[i] = ~v[i];
    }
    return res;
}

std::ostream& operator << (std::ostream &os, const BoolVector &vec) {
    for(int j=0;j<vec.m*8;j++){
        os << vec[j];
    }
    return os;
}

std::istream& operator >> (std::istream &is, BoolVector &vec) {
    std::string s;
    is >> s;
    BoolVector temp(s.c_str());
    vec = temp;
    return is;
}

class CNF {
private:
    BoolVector *pos;
    BoolVector *neg;
    std::vector<std::string> var_names;
    
    int nVar;
    int nClaus;
    
    void resizeB(int);

public:
    CNF();
    ~CNF();
    
    void addClause(int, std::string, std::string);
    void setPos(int, int);
    void setNeg(int, int);
    void unsetPos(int, int);
    void unsetNeg(int, int);
    
    //ПОТОМ УДАЛИТЬ!!!!!!!!!
    int get_nClause(){
        return nClaus;
    }
    int get_nVar(){
        return nVar;
    }
    
    void printCNF();
    void printVarNames();
    
    int findVarIndex(const std::string& name) const {
            for (int i = 0; i < var_names.size(); i++) {
                if (var_names[i] == name) {
                    return i;
                }
            }
            return -1; // не найдено
        }
    
};

CNF::CNF() {
    nVar = 0;
    nClaus = 0;
    pos = new BoolVector[1]();
    neg = new BoolVector[1]();
    var_names.push_back("nullptr");
}
CNF::~CNF () {
    delete[] pos;
    delete[] neg;
}

void CNF::addClause(int fields_num, std::string from, std::string to = "") {
    //Всё, что ниже - только для односвязного списка
    int fromIndex = findVarIndex(from);
    
    if (to == "") {
        if (fromIndex == -1) {
            //создание элемента списка и проведение связи м-ду эл-том и переменной-указателем
            var_names.push_back(from);
            var_names.push_back(from+"_f");
            nVar += 2;
            resizeB(nClaus + 2);
            setPos(nVar - 1, nVar);
            setNeg(nVar, nVar - 1);
        } else {
            //перевыделение памяти для уже существующей переменной-указателя
            int idx = -1;
            for (int i = 0; i < nVar; i++) {
                if (pos[fromIndex][i] == 1) {
                    idx = i;
                    break;
                }
            }
            neg[idx].Set0(fromIndex);
            BoolVector allZeros(nVar);
            pos[fromIndex] = pos[fromIndex] & allZeros;
            
        }
    } else {
        int toIndex = findVarIndex(to);
         
        if (fromIndex == -1) {
            var_names.push_back(from);
            nVar += 1;
            resizeB(nClaus + 1);
            if (toIndex == -1) {
                //обработка случая, когда
                /*  struct Node* head = NULL;
                    struct Node* newNode = head;
                    или
                    struct Node* head;
                    struct Node* newNode = head;
                */
                //ДОПИСАТЬ, 
            } else {
                //обработка случая, когда
                /*  struct Node* head = NULL;
                    struct Node* newNode = malloc;
                    head = newNode;
                */
                pos[nVar] = pos[fromIndex];
                neg[toIndex].Set1(nVar);
            }
        } else {
            //переопределение связи
            if (toIndex == -1) {
                
            } else {
                
            }
        }
    }
}

void CNF::setPos(int iC, int iV) {
    if (iC < nClaus) {
        pos[iC].Set1(iV);
    }
}
void CNF::setNeg(int iC, int iV) {
    if (iC < nClaus) {
        neg[iC].Set1(iV);
    }
}
void CNF::unsetPos(int iC, int iV) {
    if (iC < nClaus) {
        pos[iC].Set0(iV);
    }
}

void CNF::unsetNeg(int iC, int iV) {
    if (iC < nClaus) {
        neg[iC].Set0(iV);
    }
}

void CNF::resizeB(int new_nClaus) {
    BoolVector* new_pos = new BoolVector[new_nClaus + 1]();
    BoolVector* new_neg = new BoolVector[new_nClaus + 1]();
    
    if (pos != nullptr) {
        for (int i = 0; i < nClaus; i++) {
            new_pos[i] = pos[i];
            new_neg[i] = neg[i];
        }
    }
    for (int i = 0; i < nClaus; i++) {
        new_pos[i] = BoolVector(nVar+1) | pos[i];
    }
    for (int i = nClaus; i <= new_nClaus; i++) {
        new_pos[i] = BoolVector(nVar+1);
        new_neg[i] = BoolVector(nVar+1);
    }
    
    delete[] pos;
    delete[] neg;
        
    pos = new_pos;
    neg = new_neg;
    nClaus = new_nClaus;
}

void CNF::printCNF() {
    for (int i = 0; i<=nVar; i++) {
        std::cout<< "pos["<<i<<"]"<<pos[i]<<"    "<<"neg["<<i<<"]"<<neg[i]<<std::endl;;
    }
}

void CNF::printVarNames() {
    std::cout << "var_names (" << var_names.size() << " элементов): ";
            for (int i = 0; i < var_names.size(); i++) {
                std::cout << "[" << i << "]:" << var_names[i] << " ";
            }
            std::cout << std::endl;
}


int main() {
    
    std::ifstream data("/Users/liza/School/NIR/NIR/primer.json");
    if (!data.is_open()) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return 1;
    }
    
    json parsedJSON = json::parse(data);
    //std::cout << parsedJSON.dump(2) << std::endl;
    
    int fields_num = (int)parsedJSON[0]["fields_num"];
    CNF cnf;
    /*
     if (fields_num == 1) {
        значит список односвязный и мы обрабатываем односвязный список
     } else {
        обрабатываем двусвязный список
     }
     */
    for (int i = 1; i < parsedJSON.size(); i++) {
        if(parsedJSON[i].contains("value")){
            if(!parsedJSON[i].contains("f")) {
                std::string Val{parsedJSON[i]["value"]};
                
                if (Val == "allocated_memory") {
                    //создание элемента списка и проведение связи м-ду эл-том и переменной-указателем
                    cnf.addClause(fields_num, parsedJSON[i]["name"]);
                } else {
                    //перепроведение связи
                    cnf.addClause(fields_num, parsedJSON[i]["name"], Val);
                }
                
                
            } else {
                //отработка кейсов когда значение валью - поле перменной
            }
        }
    }
    
    cnf.printCNF();
    cnf.printVarNames();
    
    
    //строку 14 пропускаем так как там ничего не происходит
    //на строке 17 небходимо занести переменную и выделенную на нее память в таблицу
    //CNF cnf;
    //std::cout<<"1: "<<cnf.get_nClause()<<" "<<cnf.get_nVar()<<std::endl;
    //cnf.addClause(parsedJSON[2]["name"], parsedJSON[2]["value"], fields_num);
    
    /*
    std::string name{parsedJSON[0]["name"]};
    std::cout << name << std::endl;
     
    int ident{parsedJSON[0]["id"]};
    std::cout << ident << std::endl;
     
    std::string ident8{parsedJSON[8]["value"]["name"]};
    std::cout << ident8 << std::endl;
    */
    
    //CNF jsonchik;
    
    data.close();
    
    return 0;
}
