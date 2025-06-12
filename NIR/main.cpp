
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
    if (index > nbit) resizeA(m+1);
    v[index/8] |= (1 << (index%8));
}

void BoolVector::Set0(int index){
    if (index > nbit) resizeA(m+1);
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
    for(int j=0;j<vec.nbit;j++){
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
    
    void addClause(std::string, std::string, int);
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
    
};

CNF::CNF() {
    nVar = 0;
    nClaus = 0;
    pos = nullptr;
    neg = nullptr;
}
CNF::~CNF () {
    delete[] pos;
    delete[] neg;
}

void CNF::addClause(std::string from, std::string to, int fields_num) {
    bool found = std::any_of(var_names.begin(), var_names.end(), [to](std::string const& s) {return s==to;}); //проверяем, есть ли имя переменной в кнф
    if (!found){
        var_names.push_back(to);
        var_names.push_back(to+"_f");
        nVar++;
        resizeB(nClaus + 1);
        
        for (int i = 0; i < nClaus; i++) {
            pos[i].resizeA(nVar);
            neg[i].resizeA(nVar);
        }
        setPos(nClaus - 1, nVar - 1);
        std::cout<<"2: "<<nClaus<<" "<<nVar<<std::endl;
    } else {
        
    }
    //нужно определить в какой скобке уже есть исходящая связь, если она есть
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
    BoolVector* new_pos = new BoolVector[new_nClaus]();
    BoolVector* new_neg = new BoolVector[new_nClaus]();
    
    if (pos != nullptr) {
        for (int i = 0; i < nClaus; i++) {
            new_pos[i] = pos[i];
            new_neg[i] = neg[i];
        }
    }
    
    for (int i = nClaus; i < new_nClaus; i++) {
        new_pos[i] = BoolVector(nVar);
        new_neg[i] = BoolVector(nVar);
    }
    
    delete[] pos;
    delete[] neg;
        
    pos = new_pos;
    neg = new_neg;
    nClaus = new_nClaus;
}


int main() {
    
    
    std::ifstream data("/Users/liza/School/NIR/NIR/primer.json");
    if (!data.is_open()) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return 1;
    }
    
    json parsedJSON = json::parse(data);
    //std::cout << parsedJSON.dump(2) << std::endl;
    
    int fields_num = parsedJSON[0]["fields_num"];
    
    //строку 14 пропускаем так как там ничего не происходит
    //на строке 17 небходимо занести переменную и выделенную на нее память в таблицу
    CNF cnf;
    std::cout<<"1: "<<cnf.get_nClause()<<" "<<cnf.get_nVar()<<std::endl;
    cnf.addClause(parsedJSON[2]["name"], parsedJSON[2]["value"], fields_num);
    
    /*
    std::string name{parsedJSON[0]["name"]};
    std::cout << name << std::endl;
     
    int ident{parsedJSON[0]["id"]};
    std::cout << ident << std::endl;
     
    std::string ident8{parsedJSON[8]["value"]["name"]};
    std::cout << ident8 << std::endl;
    */
    
    //CNF jsonchik;
    
    std::cout << "Hello, World!\n";
    data.close();
    
    return 0;
}
