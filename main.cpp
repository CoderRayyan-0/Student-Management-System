// Student Report Management System
// Language: C++ (STL + file handling)
//
// - Subject names may contain spaces
// - Records are kept sorted by roll number, so search uses binary search
// - Roll numbers are unique
// - Saved file is loaded safely (bad lines are skipped, never a crash)
// - Unsaved changes are saved automatically on exit

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

const string FILE_NAME = "students.txt";
const int MAX_MARKS = 100;
const int MAX_SUBJECTS = 20;
const int MAX_ROLL = 9999999;

struct Student {
    string name;
    int rollNo = 0;
    map<string, int> marks; // subject -> marks
    float percentage = 0.0f;
    char grade = 'F';
};

vector<Student> students; // always kept sorted by rollNo
bool unsaved = false;     // true when there are changes not yet written to file

struct InputClosed {};    // thrown when the input stream ends (Ctrl+D / Ctrl+Z)

// ---------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------

string trim(const string& text) {
    const string spaces = " \t\r\n";
    size_t first = text.find_first_not_of(spaces);
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(spaces);
    return text.substr(first, last - first + 1);
}

string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

// Converts the WHOLE text to an int. Returns false if it is not a valid number.
bool parseInt(const string& text, int& value) {
    try {
        size_t used = 0;
        value = stoi(text, &used);
        return used == text.size();
    } catch (const exception&) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Safe input helpers (each one repeats until the input is valid)
// ---------------------------------------------------------------------------

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw InputClosed();
    return trim(line);
}

int readInt(const string& prompt, int low, int high) {
    while (true) {
        int value = 0;
        if (parseInt(readLine(prompt), value) && value >= low && value <= high)
            return value;
        cout << "Invalid input. Enter a whole number from " << low << " to " << high << ".\n";
    }
}

// Text that is saved to the file: must not be empty or contain ',' or ':'.
string readField(const string& prompt) {
    while (true) {
        string text = readLine(prompt);
        if (!text.empty() && text.find_first_of(",:") == string::npos)
            return text;
        cout << "Invalid input. Text must not be empty or contain ',' or ':'.\n";
    }
}

// ---------------------------------------------------------------------------
// Student logic
// ---------------------------------------------------------------------------

char gradeFor(float percentage) {
    if (percentage >= 90) return 'A+';
    if (percentage >= 80) return 'A';
    if (percentage >= 70) return 'B+';
    if (percentage >= 60) return 'B';
    if (percentage >= 50) return 'C+';
    if (percentage >= 40) return 'C';
    if (percentage >= 35 && percentage < 40) return 'D+';
    if (percentage >= 33 && percentage < 35) return 'D';
    return 'F';
}

void calculateGrade(Student& s) {
    if (s.marks.empty()) { // protects against dividing by zero
        s.percentage = 0.0f;
        s.grade = 'F';
        return;
    }
    int total = 0;
    for (const auto& m : s.marks)
        total += m.second;
    s.percentage = total / static_cast<float>(s.marks.size());
    s.grade = gradeFor(s.percentage);
}

// Subject names are compared ignoring case ("Maths" and "maths" are the same).
bool hasSubject(const Student& s, const string& subject) {
    string wanted = toLower(subject);
    for (const auto& m : s.marks)
        if (toLower(m.first) == wanted) return true;
    return false;
}

// ---------------------------------------------------------------------------
// Sorted storage: students is always ordered by roll number
// ---------------------------------------------------------------------------

vector<Student>::iterator lowerBoundByRoll(int roll) {
    return lower_bound(students.begin(), students.end(), roll,
                       [](const Student& s, int r) { return s.rollNo < r; });
}

// Binary search. Returns students.end() when the roll number is not present.
vector<Student>::iterator findStudent(int roll) {
    auto pos = lowerBoundByRoll(roll);
    if (pos != students.end() && pos->rollNo == roll) return pos;
    return students.end();
}

// Inserts at the correct position. Returns false if the roll number already exists.
bool insertSorted(const Student& s) {
    if (findStudent(s.rollNo) != students.end()) return false;
    students.insert(lowerBoundByRoll(s.rollNo), s);
    return true;
}

// ---------------------------------------------------------------------------
// Add student
// ---------------------------------------------------------------------------

int readRollNo() {
    while (true) {
        int roll = readInt("Enter roll number: ", 1, MAX_ROLL);
        if (findStudent(roll) == students.end()) return roll;
        cout << "Roll number " << roll << " already exists.\n";
    }
}

void readSubject(Student& s, int index) {
    string subject;
    while (true) {
        subject = readField("Enter name of subject " + to_string(index) + ": ");
        if (!hasSubject(s, subject)) break;
        cout << "That subject was already entered.\n";
    }
    s.marks[subject] = readInt("Enter marks for " + subject + " (0-" +
                               to_string(MAX_MARKS) + "): ", 0, MAX_MARKS);
}

void addStudent() {
    Student s;
    s.name = readField("Enter name: ");
    s.rollNo = readRollNo();
    int subjects = readInt("Enter number of subjects (1-" +
                           to_string(MAX_SUBJECTS) + "): ", 1, MAX_SUBJECTS);
    for (int i = 1; i <= subjects; i++)
        readSubject(s, i);

    calculateGrade(s);
    insertSorted(s);
    unsaved = true;
    cout << "Student added successfully!\n";
}

// ---------------------------------------------------------------------------
// Display and search
// ---------------------------------------------------------------------------

int nameColumnWidth() {
    size_t width = 20;
    for (const auto& s : students)
        width = max(width, s.name.size() + 2);
    return static_cast<int>(width);
}

void displayStudents() {
    if (students.empty()) {
        cout << "No student records to display.\n";
        return;
    }
    int nameWidth = nameColumnWidth();
    cout << left << setw(10) << "RollNo" << setw(nameWidth) << "Name"
         << setw(10) << "%" << setw(8) << "Grade" << "Subjects\n";
    for (const auto& s : students) {
        cout << left << setw(10) << s.rollNo << setw(nameWidth) << s.name
             << setw(10) << s.percentage << setw(8) << s.grade;
        for (const auto& m : s.marks)
            cout << m.first << ":" << m.second << " ";
        cout << "\n";
    }
}

void searchStudent() {
    int roll = readInt("Enter roll number to search: ", 1, MAX_ROLL);
    auto it = findStudent(roll);
    if (it == students.end()) {
        cout << "Student not found!\n";
        return;
    }
    cout << "Found: " << it->name << " with grade: " << it->grade
         << " and %: " << it->percentage << endl;
}

// ---------------------------------------------------------------------------
// File handling
// ---------------------------------------------------------------------------

bool saveToFile() {
    ofstream fout(FILE_NAME);
    if (!fout) {
        cout << "Error: could not open " << FILE_NAME << " for writing.\n";
        return false;
    }
    fout << fixed << setprecision(2);
    for (const auto& s : students) {
        fout << s.name << ',' << s.rollNo << ',' << s.percentage << ',' << s.grade;
        for (const auto& m : s.marks)
            fout << ',' << m.first << ':' << m.second;
        fout << '\n';
    }
    fout.close();
    if (fout.fail()) {
        cout << "Error: could not write all data to " << FILE_NAME << ".\n";
        return false;
    }
    unsaved = false;
    cout << "Data saved to file.\n";
    return true;
}

// Reads one "subject:marks" entry (e.g. "Maths:95") into the student.
bool parseSubjectEntry(const string& entry, Student& s) {
    size_t sep = entry.find(':');
    if (sep == string::npos) return false;

    string subject = trim(entry.substr(0, sep));
    int mark = 0;
    if (subject.empty() || hasSubject(s, subject)) return false;
    if (!parseInt(trim(entry.substr(sep + 1)), mark)) return false;
    if (mark < 0 || mark > MAX_MARKS) return false;

    s.marks[subject] = mark;
    return true;
}

// Reads one line: name,rollNo,percentage,grade,subject:marks,...
// Percentage and grade stored in the file are ignored and recalculated from
// the marks, so a hand-edited file can never show a wrong grade.
bool parseStudent(const string& line, Student& s) {
    stringstream ss(line);
    string rollText, savedPercentage, savedGrade;
    if (!getline(ss, s.name, ',') || !getline(ss, rollText, ',') ||
        !getline(ss, savedPercentage, ',') || !getline(ss, savedGrade, ','))
        return false;

    s.name = trim(s.name);
    if (s.name.empty()) return false;
    if (!parseInt(trim(rollText), s.rollNo) || s.rollNo < 1 || s.rollNo > MAX_ROLL)
        return false;

    string entry;
    while (getline(ss, entry, ','))
        if (!parseSubjectEntry(entry, s)) return false;

    if (s.marks.empty()) return false;
    calculateGrade(s);
    return true;
}

void loadFromFile() {
    ifstream fin(FILE_NAME);
    if (!fin) {
        cout << "No saved data found. Starting with an empty list.\n";
        return;
    }
    students.clear();
    int loaded = 0, skipped = 0;
    string line;
    while (getline(fin, line)) {
        line = trim(line);
        if (line.empty()) continue;
        Student s;
        if (parseStudent(line, s) && insertSorted(s)) loaded++;
        else skipped++;
    }
    cout << loaded << " record(s) loaded from file.";
    if (skipped > 0)
        cout << " " << skipped << " invalid or duplicate line(s) skipped"
             << " (they will be removed on the next save).";
    cout << "\n";
}

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------

void printMenu() {
    cout << "\n1. Add Student\n2. Display All\n3. Search\n4. Save to File\n5. Exit\n";
}

void runMenu() {
    int choice;
    do {
        printMenu();
        choice = readInt("Enter your choice: ", 1, 5);
        switch (choice) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudent(); break;
            case 4: saveToFile(); break;
            case 5: cout << "Exiting...\n"; break;
        }
    } while (choice != 5);
}

int main() {
    cout << fixed << setprecision(2); // percentages print with 2 decimals
    loadFromFile();
    try{
        runMenu();
    }catch(const InputClosed&){
        cout << "\nInput closed.\n";
    }
    if(unsaved) 
        saveToFile(); // never lose changes on exit

    return 0;
}
