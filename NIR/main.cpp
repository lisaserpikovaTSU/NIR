#include <iostream>
#include "json.hpp"
#include <fstream>
#include <string>
#include <iomanip>
#include <unordered_set>

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
    
    bool operator == (const BoolVector &) const;
    
    BoolVector operator | (const BoolVector &) const;
    BoolVector operator & (const BoolVector &) const;
    BoolVector operator ~() const;
    
    bool hasWeight() const;
    bool hasPrev() const;
    int getLink();
    int getLinkZeroIncluded();
    int getWeight();
    
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
    nbit = (int)strlen(str);
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
    if (index >= nbit) resizeA(index + 1);
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
    
    if (Nnbit > nbit) {
        for(int i = 0; i <= nbit; i++) {
            Nv[i] = v[i];
        }
    } else {
        for(int i = 0; i <= Nnbit; i++) {
            Nv[i] = v[i];
        }
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

bool BoolVector::operator==(const BoolVector &other) const{
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

bool BoolVector::hasWeight() const {
    for (int i = 0; i < nbit; i++) {
        if ((*this)[i] == 1) {
            return true;
        }
    }
    return false;
}

bool BoolVector::hasPrev() const {
    for (int i = 1; i < nbit; i++) {
        if ((*this)[i] == 1) {
            return true;
        }
    }
    return false;
}

int BoolVector::getLink() {
    for (int i = 1; i < nbit; i++) {
        if ((*this)[i] == 1) {
            return i;
        }
    }
    return -1;
}

int BoolVector::getLinkZeroIncluded() {
    for (int i = 0; i < nbit; i++) {
        if ((*this)[i] == 1) {
            return i;
        }
    }
    return -1;
}

int BoolVector::getWeight()  {
    int c = 0;
    for (int i = 0; i < nbit; i++) {
        if ((*this)[i] == 1) {
            c++;
        }
    }
    return c;
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
    BoolVector ptr_var;     //1 для бита индекса для тех узлов, которые являются переменными-указателями
    BoolVector *pos_type1;
    BoolVector *pos_type2;
    BoolVector *neg;
    std::vector<std::string> var_names;
    
    int nVar;
     
    void resizeB(int);

public:
    CNF();
    CNF(const CNF& other);
    ~CNF();
    CNF& operator=(const CNF&);
    
    int get_nVar(){
        return nVar;
    }
    
    std::string get_varName(int& ind) {
        return var_names[ind];
    }
    
    void printCNF();
    void printVarNames();
    
    int findVarIndex(const std::string&) const;
    int findFieldInd(int) const;
    //int findFieldIndType2(int) const;
    void deleteNode(std::string&);
    void freeNode(int&, std::vector<CNF>&, int, int);
    void bypass(int, int, std::unordered_set<int>&, std::unordered_set<int>&);
    void checkFictiousBonds(std::unordered_set<int>&, int);
    CNF divide(int);
    void addAlMem(std::string, int, int);
    void addPtrVar(std::string, int);
    void addNULL(int, int);
    void point(int&, std::string, int&);
    void rePoint(int&, int&, int&, int&);
    bool isDangling(int&, int);
    bool pointsAtNULL(int&, int);
    bool isEmpty();
    void makeDangling(int&, int);
    void merge(CNF&, int&, int&, int);
};

CNF::CNF() {
    nVar = 0;
    pos_type1 = new BoolVector[1]();
    pos_type2 = new BoolVector[1]();
    neg = new BoolVector[1]();
    ptr_var = BoolVector(1);
    /*
    pos_type1 = new BoolVector[1];
    pos_type2 = new BoolVector[1];
    neg = new BoolVector[1];
    
    pos_type1[0] = BoolVector(1);
    pos_type2[0] = BoolVector(1);
    neg[0] = BoolVector(1);
     */
    
    var_names.push_back("nullptr");
}

CNF::CNF(const CNF& other) {
    
    pos_type1 = nullptr;
    pos_type2 = nullptr;
    neg = nullptr;
    
    nVar = other.nVar;
    var_names = other.var_names;
    ptr_var = other.ptr_var;
    
    if (nVar > 0 && other.pos_type1 != nullptr && other.pos_type2 != nullptr && other.neg != nullptr) {
        pos_type1 = new BoolVector[nVar + 1];
        pos_type2 = new BoolVector[nVar + 1];
        neg = new BoolVector[nVar + 1];
        
        for (int i = 0; i <= nVar; i++) {
            pos_type1[i] = other.pos_type1[i];
            pos_type2[i] = other.pos_type2[i];
            neg[i] = other.neg[i];
        }
    } else {
        pos_type1 = new BoolVector[1]();
        pos_type2 = new BoolVector[1]();
        neg = new BoolVector[1]();
        nVar = 0;
    }
}
CNF::~CNF () {
    delete[] pos_type1;
    delete[] pos_type2;
    delete[] neg;
}

CNF& CNF::operator=(const CNF& other) {
    if (this == &other) return *this;
    
    delete[] pos_type1;
    delete[] pos_type2;
    delete[] neg;
    
    nVar = other.nVar;
    var_names = other.var_names;
    ptr_var = other.ptr_var;
    
    if (nVar > 0 && other.pos_type1 != nullptr && other.pos_type2 != nullptr && other.neg != nullptr) {
        pos_type1 = new BoolVector[nVar + 1];
        pos_type2 = new BoolVector[nVar + 1];
        neg = new BoolVector[nVar + 1];
        
        for (int i = 0; i <= nVar; i++) {
            pos_type1[i] = other.pos_type1[i];
            pos_type2[i] = other.pos_type2[i];
            neg[i] = other.neg[i];
        }
    } else {
        pos_type1 = new BoolVector[1]();
        pos_type2 = new BoolVector[1]();
        neg = new BoolVector[1]();
        nVar = 0;
    }
    
    return *this;
}

void CNF::resizeB(int new_nVar) {
    BoolVector* new_pos1 = new BoolVector[new_nVar + 1];
    BoolVector* new_pos2 = new BoolVector[new_nVar + 1];
    BoolVector* new_neg = new BoolVector[new_nVar + 1];
    
    if (new_nVar > nVar){
        for (int i = 0; i <= nVar; i++) {
            new_pos1[i] = BoolVector(nVar+1) | pos_type1[i];
            new_pos2[i] = BoolVector(nVar+1) | pos_type2[i];
            new_neg[i] = BoolVector(nVar+1) | neg[i];
        }
        for (int i = nVar+1; i <= new_nVar; i++) {
            new_pos1[i] = BoolVector(nVar+1);
            new_pos2[i] = BoolVector(nVar+1);
            new_neg[i] = BoolVector(nVar+1);
        }
    } else {
        for (int i = 0; i <= new_nVar; i++) {
            new_pos1[i] = BoolVector(nVar + 1);
            new_pos2[i] = BoolVector(nVar + 1);
            new_neg[i] = BoolVector(nVar + 1);
            
            for (int j = 0; j <= new_nVar; j++) {
                if (pos_type1[i][j] == 1) new_pos1[i].Set1(j);
                if (pos_type2[i][j] == 1) new_pos2[i].Set1(j);
                if (neg[i][j] == 1) new_neg[i].Set1(j);
            }
        }
    }
    
    
    delete[] pos_type1;
    delete[] pos_type2;
    delete[] neg;
        
    ptr_var.resizeA(new_nVar+1);
    pos_type1 = new_pos1;
    pos_type2 = new_pos2;
    neg = new_neg;
    nVar = new_nVar;
}

void CNF::printCNF() {
    for (int i = 0; i <= nVar; i++) {
        std::cout<<var_names[i]<< ' ';
    }
    std::cout<<std::endl;
    std::cout<<"ptr_var["<<ptr_var<<"]"<<std::endl;
    for (int i = 0; i<=nVar; i++) {
        std::cout<< "pos1["<<i<<"]"<<pos_type1[i]<<" "<< "pos2["<<i<<"]"<<pos_type2[i]<<" "<<"neg["<<i<<"]"<<neg[i]<<std::endl;;
    }
}

void CNF::printVarNames() {
    std::cout << "var_names (" << var_names.size() << " элементов): ";
            for (int i = 0; i < var_names.size(); i++) {
                std::cout << "[" << i << "]:" << var_names[i] << " ";
            }
            std::cout << std::endl;
}

int CNF::findVarIndex(const std::string& name) const {
    //Функция для поиска узла по имени в КНФ
    for (int i = 0; i < var_names.size(); i++) {
        if (var_names[i] == name) {
            return i;
        }
    }
    return -1;
}

int CNF::findFieldInd(int indOut) const {
    //Функция для поиска следующего узла на который ссылается узел indOut
    int ind = -1;
    for (int i = 1; i <= nVar; i++) {
        if(pos_type1[indOut][i] == 1) ind = i;
    }
    return ind;
}
/*
int CNF::findFieldIndType2(int indOut) const {
    //Функция для поиска предыдущего узла на который ссылается узел indOut
    int ind = -1;
    for (int i = 1; i <= nVar; i++) {
        if(pos_type2[indOut][i] == 1) ind = i;
    }
    return ind;
}
*/
void CNF::deleteNode(std::string& name) {
    //"Вырезаем"(сдвигаем следующие за ним узлы) удаляемый узел из строк
    int ind = findVarIndex(name);
    for (int i = ind; i <= nVar - 1; i++) {
        pos_type1[i] = pos_type1[i + 1];
        pos_type2[i] = pos_type2[i + 1];
        neg[i] = neg[i + 1];
        
        if (ptr_var[i + 1] == 1) {
            ptr_var.Set1(i);
        } else {
            ptr_var.Set0(i);
        }
    }
    ptr_var.Set0(nVar);
    
    //"Вырезаем" удаляемый узел из столбцов
    for (int i = 0; i <= nVar; i++) {
        for (int j = ind; j < nVar; j++) {
            
            if (pos_type1[i][j + 1] == 1) {
                pos_type1[i].Set1(j);
            } else {
                pos_type1[i].Set0(j);
            }
                
            if (pos_type2[i][j + 1] == 1) {
                pos_type2[i].Set1(j);
            } else {
                pos_type2[i].Set0(j);
            }
                
            if (neg[i][j + 1] == 1) {
                neg[i].Set1(j);
            } else {
                neg[i].Set0(j);
            }
        }
        pos_type1[i].Set0(nVar);
        pos_type2[i].Set0(nVar);
        neg[i].Set0(nVar);
    }
    nVar--;
    int indToDel = findVarIndex(name);
    var_names.erase(var_names.begin() + indToDel);
    
    resizeB(nVar);
}

void CNF::freeNode(int& ind, std::vector<CNF>& danglingPointers, int nFields = 1, int fieldType = 1) {
    int indToFree;
    if (fieldType == 1) {
        indToFree = pos_type1[ind].getLink();
    } else {
        indToFree = pos_type2[ind].getLink();
    }
    std::string nameToFree = var_names[indToFree];
    
    std::unordered_set<std::string> namesToDel;
    for (int i = 1; i <= nVar; i++) {
        if (neg[indToFree][i] && ptr_var[i]) {
            CNF pointer;
            pointer.addPtrVar(var_names[i], nFields);
            danglingPointers.push_back(pointer);
            namesToDel.insert(var_names[i]);
        }
    }
    for (std::string name: namesToDel) {
        deleteNode(name);
    }
    deleteNode(nameToFree);
}

void CNF::bypass(int cur, int prev, std::unordered_set<int>& list, std::unordered_set<int>& visited) {
    // Если узел уже посещен, выходим чтобы избежать зацикливания
    if (visited.count(cur)) {
        return;
    }
    
    visited.insert(cur);
    list.insert(cur);
    
    int next = pos_type1[cur].getLink();
    if (next != -1 && next != prev) {
        bypass(next, cur, list, visited);
    }
    
    int pr = pos_type2[cur].getLink();
    if (pr != -1 && pr != prev && pr != next) {
        bypass(pr, cur, list, visited);
    }
}

bool hasIntersection(const std::unordered_set<int>& set1, const std::unordered_set<int>& set2) {
    for (int elem : set1) {
        if (set2.count(elem)) return true;
    }
    return false;
}

std::vector<std::unordered_set<int>> findUnique(const std::vector<std::unordered_set<int>>& lists) {
    std::vector<std::unordered_set<int>> result;
    std::vector<bool> merged(lists.size(), false);
    
    for (int i = 0; i < lists.size(); i++) {
        if (merged[i]) continue;
        
        std::unordered_set<int> current = lists[i];
        merged[i] = true;
        
        bool changed;
        do {
            changed = false;
            for (int j = 0; j < lists.size(); j++) {
                if (!merged[j] && hasIntersection(current, lists[j])) {
                    current.insert(lists[j].begin(), lists[j].end());
                    merged[j] = true;
                    changed = true;
                }
            }
        } while (changed);
        
        result.push_back(current);
    }
    
    return result;
}

void CNF::checkFictiousBonds(std::unordered_set<int>& list, int nFields) {
    int ctr = 0;
    if (nFields == 2){
        for (int nodeInd: list) {
            if (pos_type1[nodeInd].hasWeight() && pos_type2[nodeInd].hasWeight()){
                ctr++;
            }
        }
    } else {
        for (int nodeInd: list) {
            if (pos_type1[nodeInd].hasWeight()){
                ctr++;
            }
        }
    }
    if (ctr == list.size()) {
        for (int nodeInd: list) {
            if (ptr_var[nodeInd] == 1) {
                pos_type1[0].Set1(nodeInd);
                neg[nodeInd].Set1(0);
            }
        }
    } else {
        for (int nodeInd: list) {
            if (ptr_var[nodeInd] == 1) {
                pos_type1[0].Set0(nodeInd);
                neg[nodeInd].Set0(0);
            }
        }
    }
}

CNF CNF::divide(int nFields) {
    CNF cnf;
    std::vector<std::unordered_set<int>> lists;
    for (int i = 1; i <= nVar; i++) {
        if (ptr_var[i] == 1) {
            std::unordered_set<int> list;
            std::unordered_set<int> visited;
            bypass(i, -1, list, visited);
            lists.push_back(list);
        }
    }
    std::vector<std::unordered_set<int>> listsUnique = findUnique(lists);
    //if (listsUnique.size() == 1 || listsUnique.size() == 0) {
    if( listsUnique.size() == 0) {
        std::cout<<"ПОЧЕМУ НЕТ ПЕРЕМЕННЫХ????"<<std::endl;
        return cnf;
    }
    if (listsUnique.size() == 1) {
        checkFictiousBonds(listsUnique[0], nFields);
        return cnf;
    }
    checkFictiousBonds(listsUnique[0], nFields);
    int shortInd = 0;
    for (int i = 1; i < listsUnique.size(); i++) {
        if (listsUnique[i].size() < listsUnique[shortInd].size()) {
            shortInd = i;
        }
    }
    cnf.resizeB((int)listsUnique[shortInd].size());
    
    std::vector<int> oldToNew(nVar+1, -1);
    std::unordered_set<int> newInds;
    
    int newInd = 1;
    
    for (int oldInd: listsUnique[shortInd]) {
        oldToNew[oldInd] = newInd;
        cnf.var_names.push_back(var_names[oldInd]);
        if (ptr_var[oldInd] == 1) cnf.ptr_var.Set1(newInd);
        newInds.insert(newInd);
        newInd++;
    }
    //Перенос в новую КНФ с ссылкой на null
    for (int oldFrom: listsUnique[shortInd]) {
        int newFrom = oldToNew[oldFrom];
        for (int oldTo: listsUnique[shortInd]) {
            int newTo = oldToNew[oldTo];
            
            if (pos_type1[oldFrom][oldTo] == 1) {
                cnf.pos_type1[newFrom].Set1(newTo);
            }
            if (pos_type2[oldFrom][oldTo] == 1) {
                cnf.pos_type2[newFrom].Set1(newTo);
            }
            if (neg[oldFrom][oldTo] == 1) {
                cnf.neg[newFrom].Set1(newTo);
            }
        }
        if (pos_type1[oldFrom][0] == 1){
            cnf.pos_type1[newFrom].Set1(0);
            cnf.neg[0].Set1(newFrom);
        }
        if (pos_type2[oldFrom][0] == 1){
            cnf.pos_type2[newFrom].Set1(0);
            cnf.neg[0].Set1(newFrom);
        }
    }
    std::unordered_set<std::string> namesToDel;
    for (int oldInd: listsUnique[shortInd]) {
        namesToDel.insert(var_names[oldInd]);
    }
    for (std::string name: namesToDel) {
        deleteNode(name);
    }
    
    std::unordered_set<int> newList;
    std::unordered_set<int> newVisited; // Добавлено
    for (int i = 1; i <= nVar; i++) {
        if (ptr_var[i] == 1) {
            bypass(i, -1, newList, newVisited); // Добавлен параметр visited
        }
    }
    //Перестроим фиктивные связи для оставшихся в старой кнф переменных
    for (int i = 0; i < listsUnique.size(); i++) {
        if (i != shortInd) {
            checkFictiousBonds(newList, nFields);
        }
    }
    //Перестроим фиктивные связи для переменных новой кнф
    cnf.checkFictiousBonds(newInds, nFields);
    
    return cnf;
}

void CNF::addAlMem(std::string nameIn, int indFrom, int type=1) {
    var_names.push_back(nameIn);
    resizeB(nVar+1);
    BoolVector allZ(nVar);
    if (type == 1) {
        pos_type1[indFrom] = pos_type1[indFrom] & allZ;
        pos_type1[indFrom].Set1(nVar);
    } else {
        pos_type2[indFrom] = pos_type2[indFrom] & allZ;
        pos_type2[indFrom].Set1(nVar);
    }
    
    neg[nVar].Set1(indFrom);
}

void CNF::addPtrVar(std::string name, int nFields) {
    var_names.push_back(name);
    resizeB(nVar+1);
    if (nFields == 2) {
        pos_type2[1].Set1(0);
    }
    ptr_var.Set1(1);
}

void CNF::addNULL(int indOut, int type = 1) {
    if (type == 1) {
        int prevLink = pos_type1[indOut].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){
            neg[prevLink].Set0(indOut);                         //разрываем предыдущую связь
            pos_type1[indOut].Set0(prevLink);
        }
        pos_type1[indOut].Set1(0);
    } else {
        int prevLink = pos_type2[indOut].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){
            neg[prevLink].Set0(indOut);                         //разрываем предыдущую связь
            pos_type2[indOut].Set0(prevLink);
        }
        pos_type2[indOut].Set1(0);
    }
    neg[0].Set1(indOut);
}

void CNF::point(int& ind, std::string nameOut, int& type) {
    var_names.push_back(nameOut);
    resizeB(nVar+1);
    
    pos_type1[nVar] = pos_type1[ind];
    pos_type2[nVar] = pos_type2[ind];
    neg[pos_type1[nVar].getLink()].Set1(nVar);
    
    ptr_var.Set1(nVar);
}

void CNF::rePoint(int& ind, int& out, int& type, int& nFields) {
    int prevLink;
    if (type == 1) {
        prevLink = pos_type1[out].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){                                 //>0 значит была связь в какой-то узел, но не в null
            neg[prevLink].Set0(out);                       //разрываем предыдущую связь
        }
        if (prevLink == 0) {                              //если предыдущая ссылка указывала на null
            if(nFields == 1) neg[prevLink].Set0(out);     //если список односвязный просто удаляем связь
            else if (pos_type2[out].getLinkZeroIncluded() != 0){
                neg[prevLink].Set0(out);                  //если список двусвязный и вторая ссылка не на null удалаяем связь в null
            }
        }
        pos_type1[out] = pos_type1[ind];
        neg[pos_type1[out].getLink()].Set1(out);
    } else {
        prevLink = pos_type2[out].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){                                 //>0 значит была связь в какой-то узел, но не в null
            neg[prevLink].Set0(out);                       //разрываем предыдущую связь
        }
        if (prevLink == 0 && pos_type2[out].getLinkZeroIncluded() != 0) {     //если предыдущая ссылка указывала на null
            if (pos_type1[out].getLinkZeroIncluded() != 0){
                neg[prevLink].Set0(out);                  //если список двусвязный и второй узел ссылается не на null удалаяем связь в null
            }
        }
        pos_type2[out] = pos_type1[ind];
        neg[pos_type2[out].getLink()].Set1(out);
    }
}

bool CNF::isDangling(int& ind, int type=1) {
    BoolVector allZ(nVar);
    if (type == 1) {
        if (pos_type1[ind] == allZ) return true;
    } else {
        if (pos_type2[ind] == allZ) return true;
    }
    
    return false;
}

bool CNF::pointsAtNULL(int& ind, int type = 1) {
    if (type == 1 && pos_type1[ind][0] == 1) return true;
    else if (type == 2 && pos_type2[ind][0] == 1) return true;
    return false;
}

bool CNF::isEmpty() {
    if (nVar == 0) return true;
    return false;
}

void CNF::makeDangling(int& ind, int fieldType=1) {
    BoolVector allZ(nVar+1);
    if (fieldType == 1) {
        pos_type1[ind] = pos_type1[ind] & allZ;
    } else {
        pos_type2[ind] = pos_type2[ind] & allZ;
    }
    for (int i = 1; i <= nVar; i++) {
        if (pos_type1[0][i]) {
            pos_type1[0].Set0(i);
            neg[i].Set0(0);
        }
    }
}

void CNF::merge(CNF& right, int& from, int& to, int fieldType = 1) {
    std::unordered_set<int> list;
    std::unordered_set<int> visited; // Добавлено
    for (int i = 1; i <= right.nVar; i++) {
        if (right.ptr_var[i] == 1) {
            right.bypass(i, -1, list, visited); // Добавлен параметр visited
        }
    }
    int size = (int)list.size();
    resizeB(nVar+size);
    std::vector<int> oldToNew(right.nVar+1, -1);
    std::unordered_set<int> newInds;
    
    int newInd = nVar - size + 1;
    
    std::unordered_set<std::string> namesToDel;
    
    for (int oldInd: list) {
        oldToNew[oldInd] = newInd;
        std::string name = right.var_names[oldInd];
        var_names.push_back(name);
        if (right.ptr_var[oldInd] == 1) ptr_var.Set1(newInd);
        newInds.insert(newInd);
        newInd++;
        
        namesToDel.insert(name);
    }
    for (int oldFrom: list) {
        int newFrom = oldToNew[oldFrom];
        for (int oldTo: list) {
            int newTo = oldToNew[oldTo];
            
            if (right.pos_type1[oldFrom][oldTo] == 1) {
                pos_type1[newFrom].Set1(newTo);
            }
            if (right.pos_type2[oldFrom][oldTo] == 1) {
                pos_type2[newFrom].Set1(newTo);
            }
            if (right.neg[oldFrom][oldTo] == 1) {
                neg[newFrom].Set1(newTo);
            }
        }
        if (right.pos_type1[oldFrom][0] == 1){
            pos_type1[newFrom].Set1(0);
            neg[0].Set1(newFrom);
        }
        if (right.pos_type2[oldFrom][0] == 1){
            pos_type2[newFrom].Set1(0);
            neg[0].Set1(newFrom);
        }
    }
    
    
    for (std::string name: namesToDel) {
        right.deleteNode(name);
    }
    
    int newTo = oldToNew[to];
    
    if (fieldType == 1) {
        pos_type1[from] = pos_type1[newTo];
        neg[pos_type1[from].getLink()].Set1(from);
    } else {
        pos_type2[from] = pos_type1[newTo];
        neg[pos_type2[from].getLink()].Set1(from);
    }
    
    
}

std::pair<int, int> find(std::vector<CNF>& CNFcontainer, std::string name){
    for (int i = 0; i < CNFcontainer.size(); i++) {
        int varIndex = CNFcontainer[i].findVarIndex(name);
        if (varIndex != -1) {
            return {i, varIndex};
        }
    }
    return {-1, -1};
}

void makeCNF(const json& parsedJSON, std::vector<CNF>& CNFcontainer, const std::vector<std::string>& fields) {
    int nFields = (int)fields.size();
    
    for (int i = 1; i < parsedJSON.size(); i++) {
        if (i == 45) {
            std::cout<<"Hi"<<std::endl;
        }
        //Найдем индекс кнф, в которой находится переменная с которой мы работаем, индекс этой переменной
        //Если переменной с таким именем нет вернём {-1, -1}
        std::pair<int, int> varInd = find(CNFcontainer, parsedJSON[i]["name"]);
        int type = 1;
        
        std::string name = parsedJSON[i]["name"];
        if (parsedJSON[i].contains("f")) {
            if (nFields == 2 && parsedJSON[i]["f"] == fields[1]){
                type = 2;
            }
            varInd.second = CNFcontainer[varInd.first].findFieldInd(varInd.second);
            name = CNFcontainer[varInd.first].get_varName(varInd.second);
        }
        
        std::string value;
        if (parsedJSON[i].contains("value") && !parsedJSON[i]["value"].is_structured()) {
            value = parsedJSON[i]["value"];
        }
        /*
        if (CNFcontainer.size()>0 && varInd.first!=-1) {
            CNFcontainer[varInd.first].printCNF();
        }
         */
        
        if (parsedJSON[i].contains("op") && CNFcontainer[varInd.first].get_nVar() > 1) {
            std::vector<CNF> danglingPointers;
            CNFcontainer[varInd.first].freeNode(varInd.second, danglingPointers, nFields, type);
            for (int i = 0; i < danglingPointers.size(); i++) {
                CNFcontainer.push_back(danglingPointers[i]);
            }
            if (CNFcontainer[varInd.first].get_nVar() == 0) {
                CNFcontainer.erase(CNFcontainer.begin() + varInd.first);
            } else {
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
            }

        } else if (!parsedJSON[i].contains("value")) {
            CNF nCnf;
            nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
            CNFcontainer.push_back(nCnf);
        } else if (parsedJSON[i]["value"] == "NULL") {
            if (varInd.first == -1 && varInd.second == -1) {
                CNF nCnf;
                nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
                nCnf.addNULL(1);
                std::unordered_set<int> list;
                list.insert(1);
                nCnf.checkFictiousBonds(list, nFields);
                CNFcontainer.push_back(nCnf);
            } else {
                CNFcontainer[varInd.first].addNULL(varInd.second, type);
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
            }
        } else if (value[0] == 'N') {
            if (varInd.first == -1 && varInd.second == -1) {
                CNF nCnf;
                nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
                nCnf.addAlMem(parsedJSON[i]["value"], 1, type);
                CNFcontainer.push_back(nCnf);
            } else {
                CNFcontainer[varInd.first].addAlMem(parsedJSON[i]["value"], varInd.second, type);
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
            }
        } else {
            std::pair<int, int> varInd2; //не может быть {-1, -1}, иначе код не скомпилируется
            int type2 = 1;
            std::string name2;
            if (parsedJSON[i]["value"].is_structured()) {
                varInd2 = find(CNFcontainer, parsedJSON[i]["value"]["name"]);
                if (nFields == 2 && parsedJSON[i]["value"]["f"] == fields[1]){
                    type2 = 2;
                }
                varInd2.second = CNFcontainer[varInd2.first].findFieldInd(varInd2.second);
                name2 = CNFcontainer[varInd2.first].get_varName(varInd2.second);
            } else {
                varInd2 = find(CNFcontainer, parsedJSON[i]["value"]);
                name2 = CNFcontainer[varInd2.first].get_varName(varInd2.second);
            }
            /*
            if (CNFcontainer.size()>0   && varInd2.first != -1) {
                CNFcontainer[varInd2.first].printCNF();
            }
            */
                        
            if (!parsedJSON[i].contains("f")) {       //если левая часть переменная-указатель
                if (CNFcontainer[varInd2.first].pointsAtNULL(varInd2.second, type2)) {
                    //правая часть указывает на нулл
                    if (varInd.first == -1 || CNFcontainer[varInd.first].get_nVar() > 1) {
                        if (varInd.first != -1 && CNFcontainer[varInd.first].get_nVar() > 1) {
                            CNFcontainer[varInd.first].deleteNode(name);
                        }
                        CNF nCnf;
                        nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
                        CNFcontainer.push_back(nCnf);
                        varInd.first = (int)CNFcontainer.size()-1;
                        varInd.second = 1;
                    }
                    if (!CNFcontainer[varInd.first].pointsAtNULL(varInd.second, type)) {
                        CNFcontainer[varInd.first].addNULL(varInd.second, type2);
                        std::unordered_set<int> list;
                        list.insert(1);
                        CNFcontainer[varInd.first].checkFictiousBonds(list, nFields);
                    }
                    //если левая тоже указывает на нулл, то пропускаем
                } else if (CNFcontainer[varInd2.first].isDangling(varInd2.second)) {
                    //если правая часть висячий указатель
                    if (varInd.first == -1 || CNFcontainer[varInd.first].get_nVar() > 1) {
                        if (varInd.first != -1 && CNFcontainer[varInd.first].get_nVar() > 1) {
                            CNFcontainer[varInd.first].deleteNode(name);
                        }
                        CNF nCnf;
                        nCnf.addPtrVar(parsedJSON[i]["name"], 1);
                        CNFcontainer.push_back(nCnf);
                        varInd.first = (int)CNFcontainer.size()-1;
                        varInd.second = 1;
                    }
                    if (!CNFcontainer[varInd.first].isDangling(varInd.second, type)) {
                        //если остался вариант указывает на нулл
                        CNFcontainer[varInd.first].makeDangling(varInd.second, type);
                    }
                    //если левая часть тоже висячий указатель, то пропускаем
                
                } else {    //если правая часть указывает на участок памяти
                    
                    if (varInd.first == varInd2.first){    //если они в одной кнф
                        CNFcontainer[varInd2.first].rePoint(varInd2.second, varInd.second, type, nFields);
                    }
                    else {     //если в разных
                        CNFcontainer[varInd2.first].point(varInd2.second, name, type);
                        if (varInd.first != -1) {
                            CNFcontainer[varInd.first].deleteNode(name);
                            CNF nCnf = CNFcontainer[varInd2.first].divide(nFields);
                            if (CNFcontainer[varInd.first].get_nVar() == 0) {
                                CNFcontainer.erase(CNFcontainer.begin() + varInd.first);
                            }
                        } else {
                            CNF nCnf = CNFcontainer[varInd2.first].divide(nFields);
                        }
                    }
                }
            } else {
                //левая часть обращается к полю, правая без разницы
                if (CNFcontainer[varInd2.first].pointsAtNULL(varInd2.second, type2)) {
                    CNFcontainer[varInd.first].addNULL(varInd.second, type);
                } else if (CNFcontainer[varInd2.first].isDangling(varInd2.second, type)) {
                    CNFcontainer[varInd.first].makeDangling(varInd.second, type);
                } else if (varInd.first != varInd2.first){
                    CNFcontainer[varInd.first].merge(CNFcontainer[varInd2.first], varInd.second, varInd2.second, type);
                } else {
                    CNFcontainer[varInd2.first].rePoint(varInd2.second, varInd.second, type, nFields);
                }
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
                if (CNFcontainer[varInd2.first].get_nVar() == 0) {
                    CNFcontainer.erase(CNFcontainer.begin() + varInd2.first);
                }
            }
        }
        for (int i = 0; i< CNFcontainer.size(); i++){
            CNFcontainer[i].printCNF();
        }
        std::cout<<"i: "<<i<<std::endl;
    }
}

int main() {
    std::ifstream data("/Users/liza/School/NIR/NIR/primer.json");
    if (!data.is_open()) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return 1;
    }
                            
    json parsedJSON = json::parse(data);
    //std::cout << parsedJSON.dump(2) << std::endl;
    for (int i = 1; i < parsedJSON.size(); i++) {
        std::cout<<i<<"    "<<parsedJSON[i]<<std::endl;
    }
    
    std::vector<CNF> CNFcontainer;
                             
    int fields_num = parsedJSON[0]["fields_num"];
    std::vector<std::string> fields;
    if (fields_num == 1) {
        fields.push_back(parsedJSON[0]["field"]);
    } else {
        fields.push_back(parsedJSON[0]["fields"][0]);
        fields.push_back(parsedJSON[0]["fields"][1]);
    }
    makeCNF(parsedJSON, CNFcontainer, fields);
    /*
    for (int i = 0; i < CNFcontainer.size(); i++) {
        CNFcontainer[i].printCNF();
    }
     */
                            
    data.close();
                            
    return 0;
}


