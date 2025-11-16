#include <iostream>
#include "json.hpp"
#include <fstream>
#include <string>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stack>
#include <algorithm>

using json = nlohmann::json;

using Clause = std::pair<int, int>;
using Formula = std::vector<Clause>;
// DPLL SAT Solver для произвольных формул
class DPLLSolver {
private:
    std::vector<std::vector<int>> clauses;
    std::unordered_map<int, bool> assignment;
    std::vector<int> variables;

public:
    DPLLSolver() {}
    
    // Добавить клаузу
    void addClause(const std::vector<int>& clause) {
        clauses.push_back(clause);
        // Добавляем переменные в список
        for (int lit : clause) {
            int var = std::abs(lit);
            if (std::find(variables.begin(), variables.end(), var) == variables.end()) {
                variables.push_back(var);
            }
        }
    }
    
    // Очистить все клаузы
    void clear() {
        clauses.clear();
        assignment.clear();
        variables.clear();
    }
    
    // Решить задачу
    bool solve() {
        assignment.clear();
        return dpll(clauses, assignment);
    }
    
    // Получить значение переменной в решении
    std::optional<bool> getValue(int variable) const {
        auto it = assignment.find(variable);
        if (it != assignment.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    // Получить все присваивания
    const std::unordered_map<int, bool>& getAssignment() const {
        return assignment;
    }
    
    // Получить список всех переменных
    const std::vector<int>& getVariables() const {
        return variables;
    }

private:
    bool dpll(std::vector<std::vector<int>> currentClauses,
              std::unordered_map<int, bool> currentAssignment) {
        
        // Упрощение: распространение unit clauses
        bool changed;
        do {
            changed = false;
            std::vector<std::vector<int>> newClauses;
            
            for (const auto& clause : currentClauses) {
                if (clause.size() == 1) {
                    // Unit clause
                    int lit = clause[0];
                    int var = std::abs(lit);
                    bool value = (lit > 0);
                    
                    if (currentAssignment.find(var) == currentAssignment.end()) {
                        currentAssignment[var] = value;
                        changed = true;
                    } else if (currentAssignment[var] != value) {
                        // Противоречие
                        return false;
                    }
                }
            }
            
            // Удалить удовлетворенные клаузы и противоречивые литералы
            for (const auto& clause : currentClauses) {
                std::vector<int> newClause;
                bool clauseSatisfied = false;
                
                for (int lit : clause) {
                    int var = std::abs(lit);
                    auto it = currentAssignment.find(var);
                    
                    if (it != currentAssignment.end()) {
                        bool varValue = it->second;
                        bool litValue = (lit > 0);
                        
                        if (varValue == litValue) {
                            clauseSatisfied = true;
                            break;
                        }
                        // Пропустить противоречивый литерал
                    } else {
                        newClause.push_back(lit);
                    }
                }
                
                if (!clauseSatisfied) {
                    if (newClause.empty()) {
                        // Пустая клауза - противоречие
                        return false;
                    }
                    newClauses.push_back(newClause);
                }
            }
            
            currentClauses = newClauses;
            
        } while (changed && !currentClauses.empty());
        
        // Проверка на выполнимость
        if (currentClauses.empty()) {
            assignment = currentAssignment;
            return true;
        }
        
        // Выбор переменной для ветвления (простая эвристика)
        int var = chooseVariable(currentClauses);
        if (var == -1) return false;
        
        // Попробовать присвоить true
        auto newClausesTrue = currentClauses;
        newClausesTrue.push_back({var});
        auto newAssignmentTrue = currentAssignment;
        
        if (dpll(newClausesTrue, newAssignmentTrue)) {
            return true;
        }
        
        // Попробовать присвоить false
        auto newClausesFalse = currentClauses;
        newClausesFalse.push_back({-var});
        auto newAssignmentFalse = currentAssignment;
        
        return dpll(newClausesFalse, newAssignmentFalse);
    }
    
    int chooseVariable(const std::vector<std::vector<int>>& clauses) {
        if (clauses.empty()) return -1;
        
        // Простая эвристика: выбрать первую переменную из первой не-unit клаузы
        for (const auto& clause : clauses) {
            if (clause.size() > 1) {
                for (int lit : clause) {
                    int var = std::abs(lit);
                    return var;
                }
            }
        }
        
        // Если все клаузы unit, выбрать первую переменную
        if (!clauses.empty() && !clauses[0].empty()) {
            return std::abs(clauses[0][0]);
        }
        
        return -1;
    }
};

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
    BoolVector ptr_var;       //1 на идексе бита тех объектов, которые являются переменными-указателями
    BoolVector *pos_type1;    //связь типа next
    BoolVector *pos_type2;    //связь типа prev
    BoolVector *neg;          //Входящие связи
    std::vector<std::string> var_names;
    
    int nVar;
     
    void resizeB(int);

public:
    CNF();
    CNF(const CNF& other);
    ~CNF();
    CNF& operator=(const CNF&);
    
    int get_nVar(){                         //геттер для количества объектов
        return nVar;
    }
                                       
    std::string get_varName(int& ind) {    //геттер для имен объектов
        return var_names[ind];
    }
    
    int getUnsafeLink(int& indOut, int& type) { //поиск связанного узла, который потенциально может быть потерян
        if (type == 1) {
            return pos_type1[indOut].getLink();
        } else {
            return pos_type2[indOut].getLink();
        }
    }
    
    void printCNF();
    void printVarNames();
    
    int findVarIndex(const std::string&) const;          //поиск индекса объета по имени
    int findFieldInd(int) const;                         //поиск идекса узла, к которому обращается объект с
                                                         //идексом ind
    void deleteNode(std::string&);                               //Вырезает объект из таблицы
    void freeNode(int&, std::vector<CNF>&, int, int);            //Переносит висячие указатели и удаляет объект
                                                                 //методом deleteNode
    void bypass(int, int, std::unordered_set<int>&, std::unordered_set<int>&);     //обход таблицы для отслеживания
                                                                                   //отдельных кнф
    void checkFictiousBonds(std::unordered_set<int>&, int);        //определени и переопределение фиктивнфх связей
    
    CNF divide(int);                                           //разделение кнф на несвязанные
    
    void addAlMem(std::string, int, int);                      //метод для добавления новых участков памяти
    void addPtrVar(std::string, int);                          //добавление переменных-указателей
    void addNULL(int, int);                                    //добавление ссылки на null
    void point(int&, std::string, int&);                       //добавление связи
    void rePoint(int&, int&, int&, int&);                      //перестроение связи
    
    bool isDangling(int&, int);                //проверка на то, что указатель обращается к
                                                // мусору(неинициализирован)
    bool pointsAtNULL(int&, int);              //проверка на то, что указатель обращается к null
    bool isEmpty();                            //Проверка заполнености кнф
    void makeDangling(int&, int);              //удаление исходящей ссылки
    void merge(CNF&, int&, int&, int);         //слияние двух кнф
    
    std::vector<std::vector<int>> buildCNF();
    bool isSatisfiableDPLL();
    void printCNFFormula();
    void printDPLLResult(const std::unordered_map<std::string, int>& nameToIdTable);
    void printDPLLResult();
    
    void analyzeVariableTable(const std::unordered_map<std::string, int>& nameToIdTable);
};

CNF::CNF() {
    nVar = 0;
    pos_type1 = new BoolVector[1]();         //создаем вектор длины 1, так, чтобы в кнф всегда
                                             //был один элемент - null
    pos_type2 = new BoolVector[1]();
    neg = new BoolVector[1]();               //пример - neg[0] = [0]
    ptr_var = BoolVector(1);
    
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

void CNF::resizeB(int new_nVar) {                              //для изменения размера кнф(при удалении/добалении)
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

void CNF::printCNF() {                       //вывод таблицы
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

//Функция для поиска узла по имени в КНФ
int CNF::findVarIndex(const std::string& name) const {
    for (int i = 0; i < var_names.size(); i++) {
        if (var_names[i] == name) {
            return i;
        }
    }
    return -1;
}

//Функция для поиска следующего узла на который ссылается переменная-указатель
int CNF::findFieldInd(int indOut) const {
    int ind = -1;
    for (int i = 1; i <= nVar; i++) {
        if(pos_type1[indOut][i] == 1) ind = i;
    }
    return ind;
}

//удаление узла
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
    
    int indToDel = findVarIndex(name);                //ищем по имени какой индекс удалить
    var_names.erase(var_names.begin() + indToDel);
    
    resizeB(nVar);
}

//удаление с перенос висячих указателей
void CNF::freeNode(int& ind, std::vector<CNF>& danglingPointers, int nFields = 1, int fieldType = 1) {
    int indToFree;
    
    if (fieldType == 1) {                      //ищем индес узла, который хотим освободить
        indToFree = pos_type1[ind].getLink();
    } else {
        indToFree = pos_type2[ind].getLink();
    }
    
    std::string nameToFree = var_names[indToFree];       //запоминаем его имя, т.к. при удалении других объектов
                                                         //индексы меняются
    
    std::unordered_set<std::string> namesToDel;         //Смотрим, не появилось ли висящих указателей
    for (int i = 1; i <= nVar; i++) {
        if (neg[indToFree][i] && ptr_var[i]) {
            CNF pointer;                                  //переносим в новую кнф
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

//Обходим, начиная с переменной-указателя
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

//функция для обработки списков, сформированных обходом bypass
bool hasIntersection(const std::unordered_set<int>& set1, const std::unordered_set<int>& set2) {
    for (int elem : set1) {
        if (set2.count(elem)) return true;
    }
    return false;
}

//ищем уникальные списки, для разделения кнф
std::vector<std::unordered_set<int>> findUnique(const std::vector<std::unordered_set<int>>& lists) {
    std::vector<std::unordered_set<int>> result;
    std::vector<bool> merged(lists.size(), false);
    
    for (int i = 0; i < lists.size(); i++) {
        if (merged[i]) continue;                 //если объект уже попадался
        
        std::unordered_set<int> current = lists[i];
        merged[i] = true;
        
        bool changed;
        do {
            changed = false;
            for (int j = 0; j < lists.size(); j++) {
                if (!merged[j] && hasIntersection(current, lists[j])) {
                    current.insert(lists[j].begin(), lists[j].end());       //добавляем очередной объект в список
                    merged[j] = true;
                    changed = true;
                }
            }
        } while (changed);
        
        result.push_back(current);
    }
    
    return result;
}

//Проверка и корректировка фиктивных связей
void CNF::checkFictiousBonds(std::unordered_set<int>& list, int nFields) {
    int ctr = 0;
    //для формирования фиктивных связей нужно чтобы каждый объект указывал куда-либо(узел или null)
    if (nFields == 2){     //если работаем с двусвязным
        for (int nodeInd: list) {
            if (pos_type1[nodeInd].hasWeight() && pos_type2[nodeInd].hasWeight()){
                ctr++;              //для двусвязного списка вектор pos_type2 для переменных-указателей сразу инициализируется с ссылкой на null
            }
        }
    } else {               //если работаем с односвязным
        for (int nodeInd: list) {
            if (pos_type1[nodeInd].hasWeight()){
                ctr++;
            }
        }
    }
    if (ctr == list.size()) {
        for (int nodeInd: list) {             //Если все объекты "закрыты", то мы проводим фиктивные связи из null в переменные-указатели
            if (ptr_var[nodeInd] == 1) {
                pos_type1[0].Set1(nodeInd);
                neg[nodeInd].Set1(0);
            }
        }
    } else {
        for (int nodeInd: list) {            //Иначе мы удаляем все фиктивные связи
            if (ptr_var[nodeInd] == 1) {
                pos_type1[0].Set0(nodeInd);
                neg[nodeInd].Set0(0);
            }
        }
    }
}

//для разделения кнф
CNF CNF::divide(int nFields) {
    CNF cnf;
    std::vector<std::unordered_set<int>> lists;     //для каждой переменной-указателся составляем списки объектов, достижимых из нее
    for (int i = 1; i <= nVar; i++) {
        if (ptr_var[i] == 1) {
            std::unordered_set<int> list;
            std::unordered_set<int> visited;
            bypass(i, -1, list, visited);
            lists.push_back(list);
        }
    }
    std::vector<std::unordered_set<int>> listsUnique = findUnique(lists);    //т.к. эти списки могут пересекаться, находим пересечения и составляем уникальные списки
    
    if( listsUnique.size() == 0) {
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
    
    for (int oldInd: listsUnique[shortInd]) {               //переопределяем индексы
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
    
    std::unordered_set<std::string> namesToDel;         //Удаляем перенесенные узлы из старой кнф
    for (int oldInd: listsUnique[shortInd]) {
        namesToDel.insert(var_names[oldInd]);
    }
    for (std::string name: namesToDel) {
        deleteNode(name);
    }
    
    std::unordered_set<int> newList;
    std::unordered_set<int> newVisited;
    for (int i = 1; i <= nVar; i++) {
        if (ptr_var[i] == 1) {
            bypass(i, -1, newList, newVisited);
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

//добаление участка памяти в кнф
void CNF::addAlMem(std::string nameIn, int indFrom, int type=1) {
    var_names.push_back(nameIn);
    
    resizeB(nVar+1);
    
    BoolVector allZ(nVar);
    if (type == 1) {                                        //если node->next = malloc или node = malloc
        pos_type1[indFrom] = pos_type1[indFrom] & allZ;                         //удаляем старую связь из узла с индексом indFrom
        pos_type1[indFrom].Set1(nVar);
    } else {                                                //если node->prev = malloc
        neg[pos_type1[indFrom].getLinkZeroIncluded()].Set0(indFrom);
        pos_type2[indFrom] = pos_type2[indFrom] & allZ;
        pos_type2[indFrom].Set1(nVar);
    }
    
    neg[nVar].Set1(indFrom);
}

//добавление переменной-указателей    случай node* var; (объявление без инициализации)
void CNF::addPtrVar(std::string name, int nFields) {
    var_names.push_back(name);
    resizeB(nVar+1);
    if (nFields == 2) {
        pos_type2[1].Set1(0);
    }
    ptr_var.Set1(1);
}

//добавление null
void CNF::addNULL(int indOut, int type = 1) {
    if (type == 1) {             //var = null; или var->next = null;
        int prevLink = pos_type1[indOut].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){
            neg[prevLink].Set0(indOut);                         //разрываем предыдущую связь
            pos_type1[indOut].Set0(prevLink);
        }
        pos_type1[indOut].Set1(0);
    } else {                    //var->prev = null;
        int prevLink = pos_type2[indOut].getLinkZeroIncluded();   //ищем предыдущую связь
        if (prevLink > 0){
            neg[prevLink].Set0(indOut);                         //разрываем предыдущую связь
            pos_type2[indOut].Set0(prevLink);
        }
        pos_type2[indOut].Set1(0);
    }
    neg[0].Set1(indOut);
}


void CNF::point(int& ind, std::string nameOut, int& type) {       //var = ...(только если левая часть не имеет полей)
    var_names.push_back(nameOut);
    resizeB(nVar+1);
    
    pos_type1[nVar] = pos_type1[ind];
    pos_type2[nVar] = pos_type2[ind];
    neg[pos_type1[nVar].getLink()].Set1(nVar);
    
    ptr_var.Set1(nVar);
}

//переопредение связей
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

//для проверки, указывает ли объект на что-либо
bool CNF::isDangling(int& ind, int type=1) {
    BoolVector allZ(nVar);
    if (type == 1) {
        if (pos_type1[ind] == allZ) return true;
    } else {
        if (pos_type2[ind] == allZ) return true;
    }
    
    return false;
}

//проверка, указывает ли на null
bool CNF::pointsAtNULL(int& ind, int type = 1) {
    if (type == 1 && pos_type1[ind][0] == 1) return true;
    else if (type == 2 && pos_type2[ind][0] == 1) return true;
    return false;
}

//проверка, если в кнф переменные
bool CNF::isEmpty() {
    if (nVar == 0) return true;
    return false;
}

//удалить связь
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

//слияние кнф
void CNF::merge(CNF& right, int& from, int& to, int fieldType = 1) {
    std::unordered_set<int> list;
    std::unordered_set<int> visited;                //обходим список, так как в правой кнф могут быть потерянные узлы, которые при обходе не войдут в список
    for (int i = 1; i <= right.nVar; i++) {         //для переноса
        if (right.ptr_var[i] == 1) {
            right.bypass(i, -1, list, visited);
        }
    }
    int size = (int)list.size();
    resizeB(nVar+size);
    std::vector<int> oldToNew(right.nVar+1, -1);             //соотношение старых индексов в старой кнф с новыми индексами в новой
    std::unordered_set<int> newInds;
    
    int newInd = nVar - size + 1;
    
    std::unordered_set<std::string> namesToDel;
    
    for (int oldInd: list) {                                    //определяем соотношение старых-новых индексов
        oldToNew[oldInd] = newInd;
        std::string name = right.var_names[oldInd];
        var_names.push_back(name);
        if (right.ptr_var[oldInd] == 1) ptr_var.Set1(newInd);
        newInds.insert(newInd);
        newInd++;
        
        namesToDel.insert(name);
    }
    
    for (int oldFrom: list) {                               //переносим связи
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
    
    
    for (std::string name: namesToDel) {        //удаляем объекты из старой кнф
        right.deleteNode(name);
    }
    
    int newTo = oldToNew[to];
    
    if (fieldType == 1) {             //если var = .. или var->next = ..
        neg[0].Set0(from);
        pos_type1[from] = pos_type1[newTo];
        neg[pos_type1[from].getLink()].Set1(from);
    } else {                          //если var->prev = ..
        neg[0].Set0(from);
        pos_type2[from] = pos_type1[newTo];
        neg[pos_type2[from].getLink()].Set1(from);
    }
    
    
}

//поиск объекта по имени: индекс кнф, в котором он находится и индекс объекта
std::pair<int, int> find(std::vector<CNF>& CNFcontainer, std::string name){
    for (int i = 0; i < CNFcontainer.size(); i++) {
        int varIndex = CNFcontainer[i].findVarIndex(name);
        if (varIndex != -1) {
            return {i, varIndex};
        }
    }
    return {-1, -1};
}

//обработка json-файла, алгоритм формирования булевой таблицы кнф
void makeBoolLinks(const json& parsedJSON, std::vector<CNF>& CNFcontainer, const std::vector<std::string>& fields, std::unordered_map<std::string, int>& nameToIdTable) {
    int nFields = (int)fields.size();
    
    //for (int i = 1; i < parsedJSON.size(); i++) {
    for (int i = 1; i < 25; i++) {               //стоппер для проверки промежуточных значений
    //    if (i == 25) {
    //        std::cout<<"i"<<std::endl;
    //    }
        //Найдем индекс кнф, в которой находится переменная с которой мы работаем, индекс этой переменной
        //Если переменной с таким именем нет вернём {-1, -1}
        std::pair<int, int> varInd = find(CNFcontainer, parsedJSON[i]["name"]);
        int type = 1;     //тип поля с которым мы работаем - 1 для указателей и поля next, 2 для поля prev
        
        //var->NODE1->(next) NODE2
        //если нет обращения к полю мы работаем с var(определяем связи для неё, меняя первую стрелку)
        //если есть обращение к полю работаем с NODE1 и определяем связи для неё, меняя вторую стрелку
        
        std::string name = parsedJSON[i]["name"];
        if (parsedJSON[i].contains("f")) {                    //если есть обращение к полю, ищем индекс NODE1
            if (nFields == 2 && parsedJSON[i]["f"] == fields[1]){
                type = 2;
            }
            varInd.second = CNFcontainer[varInd.first].findFieldInd(varInd.second);
            name = CNFcontainer[varInd.first].get_varName(varInd.second);
        }
        
        //добавляем в таблицу соответствий имя переменной и номер строки кода
        nameToIdTable[name] = parsedJSON[i]["id"];
        if (varInd.first != -1) {
            int next = CNFcontainer[varInd.first].getUnsafeLink(varInd.second, type);  //найдем на какой узел ссылается переменная, с которой мы работаем, потому что у нас есть возможность потерять этот узел
            if (next != -1) {
                nameToIdTable[CNFcontainer[varInd.first].get_varName(next)] = parsedJSON[i]["id"]; //перезапишем номер строки для узла
            }
        }
        
        std::string value;
        if (parsedJSON[i].contains("value") && !parsedJSON[i]["value"].is_structured()) { //если правая часть var/null
            value = parsedJSON[i]["value"];
        }
        
        if (parsedJSON[i].contains("op") && CNFcontainer[varInd.first].get_nVar() > 1) { //free
            std::vector<CNF> danglingPointers;
            //удаляем узел
            CNFcontainer[varInd.first].freeNode(varInd.second, danglingPointers, nFields, type);
            //если остались висячие указатели - переносим их по разным новым кнф(они будут единственными объектами)
            for (int i = 0; i < danglingPointers.size(); i++) {
                CNFcontainer.push_back(danglingPointers[i]);
            }
            //удаляем висячие указатели из старой кнф
            if (CNFcontainer[varInd.first].get_nVar() == 0) {
                CNFcontainer.erase(CNFcontainer.begin() + varInd.first);
            } else {
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
            }

        } else if (!parsedJSON[i].contains("value")) {     //node* var;
            CNF nCnf;
            nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
            CNFcontainer.push_back(nCnf);
            
        } else if (parsedJSON[i]["value"] == "NULL") {          //var = NULL;
            if (varInd.first == -1 && varInd.second == -1) {    //если имени переменной не содержится ни в одной кнф
                CNF nCnf;
                nCnf.addPtrVar(parsedJSON[i]["name"], nFields);   //создаем новую
                nCnf.addNULL(1);                                    //указываем на null
                std::unordered_set<int> list;
                list.insert(1);
                nCnf.checkFictiousBonds(list, nFields);             //проводим фиктивные связи
                CNFcontainer.push_back(nCnf);
            } else {
                CNFcontainer[varInd.first].addNULL(varInd.second, type);    //если есть указываем на null
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);      //разделяем
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);      //если разделять нечего - nCnf окажется пустой
            }
        } else if (value[0] == 'N') {                            //если значение - выделенная память (в моем формате json'а память N1, N2 и тд)
            if (varInd.first == -1 && varInd.second == -1) {     //node* var = malloc;
                CNF nCnf;
                nCnf.addPtrVar(parsedJSON[i]["name"], nFields);   //создаем новую кнф
                nCnf.addAlMem(parsedJSON[i]["value"], 1, type);   //заносим узел
                CNFcontainer.push_back(nCnf);
            } else {
                CNFcontainer[varInd.first].addAlMem(parsedJSON[i]["value"], varInd.second, type);  //добавляем узел к кнф указателя
                CNF nCnf = CNFcontainer[varInd.first].divide(nFields);               //разделяем, там же переопределяются фиктивне связи
                if (nCnf.get_nVar() > 0) CNFcontainer.push_back(nCnf);
            }
            nameToIdTable[value] = parsedJSON[i]["id"]; //Добавим новый узел в таблицу соответствий
        } else {
            std::pair<int, int> varInd2; //не может быть {-1, -1}, иначе исходный код не скомпилируется
            int type2 = 1;   //тип обращения к объекту для правой части
            std::string name2;
            if (parsedJSON[i]["value"].is_structured()) {                         //если .. = var->next / .. = var->prev
                //var->Node1->Node2
                //когда работаем с правой частью мы ссылаемся на узел, а на на переменную, поэтому при ..=var мы работаем не с индексом var, а с индексом Node1
                varInd2 = find(CNFcontainer, parsedJSON[i]["value"]["name"]);
                if (nFields == 2 && parsedJSON[i]["value"]["f"] == fields[1]){
                    type2 = 2;
                }
                //а если .. = var->next мы работаем с индексом Node2
                varInd2.second = CNFcontainer[varInd2.first].findFieldInd(varInd2.second);
                name2 = CNFcontainer[varInd2.first].get_varName(varInd2.second);
            } else {
                varInd2 = find(CNFcontainer, parsedJSON[i]["value"]);
                name2 = CNFcontainer[varInd2.first].get_varName(varInd2.second);
            }
            
            //добавляем в таблицу соответствий имя переменной и номер строки кода
            nameToIdTable[name2] = parsedJSON[i]["id"];
            int next = CNFcontainer[varInd2.first].getUnsafeLink(varInd2.second, type2);  //найдем на какой узел ссылается переменная, с которой мы работаем, потому что у нас есть возможность потерять этот узел
            if (next != -1) {
                nameToIdTable[CNFcontainer[varInd2.first].get_varName(next)] = parsedJSON[i]["id"]; //перезапишем номер строки для узла
            }
                        
            if (!parsedJSON[i].contains("f")) {       //если var = ..
                if (CNFcontainer[varInd2.first].pointsAtNULL(varInd2.second, type2)) { //Например, var = var2->next             var2->node->null
                    if (varInd.first == -1 || CNFcontainer[varInd.first].get_nVar() > 1) {
                        if (varInd.first != -1 && CNFcontainer[varInd.first].get_nVar() > 1) {
                            CNFcontainer[varInd.first].deleteNode(name);       //если левая часть есть в какой-то кнф, удаляем её из этой кнф
                        }
                        CNF nCnf;
                        nCnf.addPtrVar(parsedJSON[i]["name"], nFields);
                        CNFcontainer.push_back(nCnf);
                        varInd.first = (int)CNFcontainer.size()-1;
                        varInd.second = 1;
                    }
                    if (!CNFcontainer[varInd.first].pointsAtNULL(varInd.second, type)) { //если левая часть не показывает на null
                        CNFcontainer[varInd.first].addNULL(varInd.second, type2);
                        std::unordered_set<int> list;
                        list.insert(1);
                        CNFcontainer[varInd.first].checkFictiousBonds(list, nFields);
                    }
                    //если левая тоже указывает на null, то пропускаем
                    
                } else if (CNFcontainer[varInd2.first].isDangling(varInd2.second)) {  //если var = var2       var2->node->
                    //если правая часть висячий указатель
                    if (varInd.first == -1 || CNFcontainer[varInd.first].get_nVar() > 1) {
                        if (varInd.first != -1 && CNFcontainer[varInd.first].get_nVar() > 1) {
                            CNFcontainer[varInd.first].deleteNode(name);                            //удаляем левый из старой кнф
                        }
                        CNF nCnf;
                        nCnf.addPtrVar(parsedJSON[i]["name"], 1);                //переносим указатель в новую кнф
                        CNFcontainer.push_back(nCnf);
                        varInd.first = (int)CNFcontainer.size()-1;
                        varInd.second = 1;
                    }
                    if (!CNFcontainer[varInd.first].isDangling(varInd.second, type)) {    //var-> null
                        //если остался вариант указывает на нулл
                        CNFcontainer[varInd.first].makeDangling(varInd.second, type);
                    }
                    //если левая часть тоже висячий указатель, то ничего не делаем
                
                } else {    //если правая часть указывает на участок памяти
                    if (varInd.first == varInd2.first){    //если они в одной кнф
                        CNFcontainer[varInd2.first].rePoint(varInd2.second, varInd.second, type, nFields); //переносим связь
                    }
                    else {     //если в разных
                        CNFcontainer[varInd2.first].point(varInd2.second, name, type);        //добавляем указатель в кнф правой части
                        if (varInd.first != -1) {
                            CNFcontainer[varInd.first].deleteNode(name);                       //удаляем из старой кнф и разделяем
                            CNF nCnf = CNFcontainer[varInd2.first].divide(nFields);
                            if (CNFcontainer[varInd.first].get_nVar() == 0) {
                                CNFcontainer.erase(CNFcontainer.begin() + varInd.first);       //если старая кнф осталсь пустой, удаляем её
                            }
                        }
                    }
                }
            } else {
                //левая часть обращается к полю, правая без разницы           var->next = ..
                if (CNFcontainer[varInd2.first].pointsAtNULL(varInd2.second, type2)) {         //var2->null или var2->node->null
                    CNFcontainer[varInd.first].addNULL(varInd.second, type);
                } else if (CNFcontainer[varInd2.first].isDangling(varInd2.second, type)) {      //var2->    или var2->node->
                    CNFcontainer[varInd.first].makeDangling(varInd.second, type);
                } else if (varInd.first != varInd2.first){          //левая и правая часть в разных кнф  и var2->node или var2->node->node1
                    CNFcontainer[varInd.first].merge(CNFcontainer[varInd2.first], varInd.second, varInd2.second, type);
                } else {                                            //в одной кнф   и var2->node или var2->node->node1
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

std::vector<std::vector<int>> CNF::buildCNF() {
    std::vector<std::vector<int>> cnf;
    std::unordered_set<int> used_variables;
        
    // Основной алгоритм построения КНФ из графа связей
    for (int i = 0; i <= nVar; i++) {
        for (int j = 0; j <= nVar; j++) {
            if (neg[j][i] == 1) {
                // Каждое отношение neg[j][i] дает клаузу: -i ∨ j
                std::vector<int> clause;
                clause.push_back(-(i + 1)); // +1 чтобы избежать 0
                clause.push_back(j + 1);
                cnf.push_back(clause);
                    
                used_variables.insert(i + 1);
                used_variables.insert(j + 1);
            }
        }
    }
    
    // Добавляем черно-белые дизъюнкты
        
    // Белый дизъюнкт: содержит все переменные (x1 ∨ x2 ∨ x3 ∨ ...)
    if (nVar > 0) {
        std::vector<int> white_clause;
        for (int i = 1; i <= nVar + 1; i++) { // +1 потому что мы сдвинули индексы
            white_clause.push_back(i);
        }
        cnf.push_back(white_clause);
    }
        
    // Черный дизъюнкт: содержит все переменные с отрицанием (¬x1 ∨ ¬x2 ∨ ¬x3 ∨ ...)
    if (nVar > 0) {
        std::vector<int> black_clause;
        for (int i = 1; i <= nVar + 1; i++) { // +1 потому что мы сдвинули индексы
            black_clause.push_back(-i);
        }
        cnf.push_back(black_clause);
    }
    
    return cnf;
}

bool CNF::isSatisfiableDPLL() {
    std::vector<std::vector<int>> cnf = buildCNF();
        
    if (cnf.empty()) {
        return true;
    }
        
    DPLLSolver solver;
    for (const auto& clause : cnf) {
        solver.addClause(clause);
    }
        
    return solver.solve();
}

void CNF::printCNFFormula() {
    std::vector<std::vector<int>> cnf = buildCNF();
        
    std::cout << "CNF формула:" << std::endl;
    for (size_t i = 0; i < cnf.size(); i++) {
        const auto& clause = cnf[i];
        std::cout << "(";
        for (size_t j = 0; j < clause.size(); j++) {
            int lit = clause[j];
            if (lit > 0) {
                std::cout << "x" << lit;
            } else {
                std::cout << "¬x" << -lit;
            }
            if (j < clause.size() - 1) {
                std::cout << " ∨ ";
            }
        }
        std::cout << ")";
        if (i < cnf.size() - 1) {
            std::cout << " ∧ ";
        }
        std::cout << std::endl;
    }
}

void CNF::printDPLLResult(const std::unordered_map<std::string, int>& nameToIdTable) {
    std::vector<std::vector<int>> cnf = buildCNF();
    
    std::cout << "CNF формула: " << std::endl;
    for (size_t i = 0; i < cnf.size(); i++) {
        const auto& clause = cnf[i];
        std::cout << "(";
        for (size_t j = 0; j < clause.size(); j++) {
            std::cout << clause[j];
            if (j < clause.size() - 1) std::cout << " ∨ ";
        }
        std::cout << ")";
        if (i < cnf.size() - 1) std::cout << " ∧ ";
    }
    std::cout << std::endl;
    
    bool satisfiable = isSatisfiableDPLL();
    std::cout << "DPLL SAT результат: ";
    if (satisfiable) {
        std::cout << "SATISFIABLE" << std::endl;
        // Вызываем отдельную функцию анализа
        analyzeVariableTable(nameToIdTable);
    } else {
        std::cout << "UNSATISFIABLE" << std::endl;
    }
    std::cout << std::endl;
}

void CNF::analyzeVariableTable(const std::unordered_map<std::string, int>& nameToIdTable) {
    std::vector<std::vector<int>> cnf = buildCNF();
    
    // Собираем статистику по переменным
    std::unordered_map<int, int> var_positive_count;
    std::unordered_map<int, int> var_negative_count;
    std::unordered_set<int> all_variables;
    
    // Исключаем черно-белые дизъюнкты из анализа
    std::vector<std::vector<int>> non_bw_cnf;
    for (const auto& clause : cnf) {
        bool is_white_clause = true;
        bool is_black_clause = true;
        
        if (clause.size() == nVar + 1) {
            for (int lit : clause) {
                if (lit <= 0) {
                    is_white_clause = false;
                    break;
                }
            }
        } else {
            is_white_clause = false;
        }
        
        if (clause.size() == nVar + 1) {
            for (int lit : clause) {
                if (lit >= 0) {
                    is_black_clause = false;
                    break;
                }
            }
        } else {
            is_black_clause = false;
        }
        
        if (!is_white_clause && !is_black_clause) {
            non_bw_cnf.push_back(clause);
            
            for (int lit : clause) {
                int var = std::abs(lit);
                all_variables.insert(var);
                
                if (lit > 0) {
                    var_positive_count[var]++;
                } else {
                    var_negative_count[var]++;
                }
            }
        }
    }
    
    // Анализируем каждую переменную
    std::vector<int> problematic_vars;
    std::vector<int> only_positive_vars;
    std::vector<int> only_negative_vars;
    
    for (int var : all_variables) {
        int pos_count = var_positive_count[var];
        int neg_count = var_negative_count[var];
        
        if (pos_count == 0 && neg_count == 0) {
            problematic_vars.push_back(var);
        }
        else if (pos_count == 0 && neg_count > 0) {
            problematic_vars.push_back(var);
            only_negative_vars.push_back(var);
        }
        else if (pos_count > 0 && neg_count == 0) {
            only_positive_vars.push_back(var);
        }
    }
    
    // Выводим общую статистику
    std::cout << "--- Анализ таблицы переменных ---" << std::endl;
    // Детальный анализ проблемных переменных
    if (!problematic_vars.empty()) {
        std::cout << "--- Проблемные переменные ---" << std::endl;
        std::cout << "  Индексы: ";
        for (size_t i = 0; i < problematic_vars.size(); i++) {
            std::cout << problematic_vars[i];
            if (i < problematic_vars.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        
        std::cout << "  Соответствие именам и строкам кода:" << std::endl;
        for (int var : problematic_vars) {
            int original_index = var - 1;
            if (original_index >= 0 && original_index < var_names.size()) {
                std::string var_name = var_names[original_index];
                std::cout << "    - " << var_name;
                
                // Ищем в таблице соответствий
                auto it = nameToIdTable.find(var_name);
                if (it != nameToIdTable.end()) {
                    std::cout << " -> строка " << it->second;
                    
                } else {
                    std::cout << " -> строка не найдена в таблице";
                }
                std::cout << std::endl;
            } else {
                std::cout << "    - x" << var << " -> имя не найдено в var_names" << std::endl;
            }
        }
    }
}


int main() {
    std::ifstream data("/Users/liza/School/NIR/NIR/primer.json");
    if (!data.is_open()) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return 1;
    }
    
    //парсим json
    json parsedJSON = json::parse(data);
    
    std::vector<CNF> CNFcontainer;
                             
    //определяем тип - односвязный или двусвязный
    int fields_num = parsedJSON[0]["fields_num"];
    std::vector<std::string> fields;
    if (fields_num == 1) {
        fields.push_back(parsedJSON[0]["field"]);
    } else {
        fields.push_back(parsedJSON[0]["fields"][0]);
        fields.push_back(parsedJSON[0]["fields"][1]);
    }
    //таблица соответствия
    std::unordered_map<std::string, int> nameToIdTable;
    //строим булеву таблицу
    makeBoolLinks(parsedJSON, CNFcontainer, fields, nameToIdTable);
    
    std::cout << "\n=== РЕЗУЛЬТАТЫ SAT ПРОВЕРКИ ===" << std::endl;
        for (int i = 0; i < CNFcontainer.size(); i++) {
            std::cout << "\n=== CNF " << i + 1 << " ===" << std::endl;
            CNFcontainer[i].printCNF();
                
            std::cout << "--- SAT Анализ ---" << std::endl;
            CNFcontainer[i].printDPLLResult(nameToIdTable);
        }

                            
    data.close();
                            
    return 0;
}


