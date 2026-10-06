# Student Report Management System

A console-based C++ application for managing student academic records. It stores subject-wise marks for each student, calculates the percentage and grade automatically, keeps records sorted by roll number, and saves them to a text file so they are available the next time the program starts.

## Table of Contents

1. [Features](#1-features)
2. [Build and Run](#2-build-and-run)
3. [Sample Run](#3-sample-run)
4. [Project Structure](#4-project-structure)
5. [Data Structures](#5-data-structures)
6. [How the Program Works](#6-how-the-program-works)
7. [Input Rules](#7-input-rules)
8. [Grading](#8-grading)
9. [File Format and Loading](#9-file-format-and-loading)
10. [Function Reference](#10-function-reference)
11. [Complexity](#11-complexity)
12. [Limitations](#12-limitations)
13. [Possible Improvements](#13-possible-improvements)
14. [Concepts Demonstrated](#14-concepts-demonstrated)

---

## 1. Features

- Add a student with a name, a roll number and marks for 1 to 20 subjects
- Calculate the percentage and grade automatically
- Display all students in a formatted table
- Search for a student by roll number using **binary search**
- Keep records **sorted by roll number** at all times and reject duplicate roll numbers
- Validate every input, so wrong input never crashes the program or causes an endless loop
- Save records to `students.txt` and load them again on the next start
- **Save automatically on exit** if there are unsaved changes
- Load the file safely: invalid lines are skipped and reported instead of crashing the program

---

## 2. Build and Run

### Requirements

- A C++ compiler such as `g++` or MinGW
- C++11 or later

### Compile

```bash
g++ -std=c++11 -Wall -Wextra main.cpp -o student_report
```

### Run

```bash
# Windows
.\student_report.exe

# Linux / macOS
./student_report
```

The program reads and writes `students.txt` in the folder it is run from. The file does not need to exist beforehand: if it is missing, the program starts with an empty list and creates the file on the first save.

---

## 3. Sample Run

Typed input is shown after each prompt.

```text
2 record(s) loaded from file.

1. Add Student
2. Display All
3. Search
4. Save to File
5. Exit
Enter your choice: 1
Enter name: Ravi Kumar
Enter roll number: 150
Enter number of subjects (1-20): 2
Enter name of subject 1: Computer Science
Enter marks for Computer Science (0-100): 150
Invalid input. Enter a whole number from 0 to 100.
Enter marks for Computer Science (0-100): 95
Enter name of subject 2: Maths
Enter marks for Maths (0-100): 70
Student added successfully!

1. Add Student
2. Display All
3. Search
4. Save to File
5. Exit
Enter your choice: 2
RollNo    Name                %         Grade   Subjects
100       Arjun               90.33     A       Chemistry:89 Maths:98 Physics:84
150       Ravi Kumar          82.50     B       Computer Science:95 Maths:70
201       Sweita              80.67     B       bio:86 chemistry:91 physics:65

1. Add Student
2. Display All
3. Search
4. Save to File
5. Exit
Enter your choice: 3
Enter roll number to search: 100
Found: Arjun with grade: A and %: 90.33

1. Add Student
2. Display All
3. Search
4. Save to File
5. Exit
Enter your choice: 5
Exiting...
Data saved to file.
```

---

## 4. Project Structure

```text
Student-Report-Management-System/
│
├── main.cpp        Complete source code
├── students.txt    Saved records (created automatically)
└── README.md       This document
```

---

## 5. Data Structures

Each student is a `struct`:

```cpp
struct Student {
    string name;
    int rollNo = 0;
    map<string, int> marks; // subject -> marks
    float percentage = 0.0f;
    char grade = 'F';
};
```

All students live in one global vector, plus a flag that tracks unsaved changes:

```cpp
vector<Student> students; // always kept sorted by rollNo
bool unsaved = false;     // true when there are changes not yet written to file
```

| Member | Type | Purpose |
|--------|------|---------|
| `name` | `string` | Student's name |
| `rollNo` | `int` | Unique roll number (1 to 999999) |
| `marks` | `map<string,int>` | Subject name to marks |
| `percentage` | `float` | Average of all subject marks |
| `grade` | `char` | A, B, C, D or F |

**Why `map` for marks?** Subject names are unique keys, so a subject cannot be stored twice, and the subjects stay ordered by name. That is why they appear in alphabetical order in the table and in the file. Ordering is by character code, so uppercase letters come before lowercase ones (`Maths` is listed before `bio`).

Example of one student in memory:

```text
Student
├── name:       Arjun
├── rollNo:     100
├── percentage: 90.33
├── grade:      A
└── marks
    ├── Chemistry → 89
    ├── Maths     → 98
    └── Physics   → 84
```

---

## 6. How the Program Works

### Program flow

```text
                 START
                   │
                   ▼
            loadFromFile()
                   │
                   ▼
        ┌─────► Show menu ◄──────────────────┐
        │          │                         │
        │   Read choice (1-5)                │
        │          │                         │
        │   ┌──────┼────────┬─────────┐      │
        │   ▼      ▼        ▼         ▼      │
        │  Add  Display   Search     Save    │
        │   │      │        │         │      │
        │   └──────┴────────┴─────────┴──────┘
        │                   (back to menu)
        │
        └── choice 5, or input stream closed
                   │
                   ▼
         unsaved changes? ── yes ──► saveToFile()
                   │
                   ▼
                  END
```

### Sorted storage and binary search

The vector is always ordered by roll number. New students are inserted at the correct position instead of being added at the end, so the vector never needs to be sorted again.

```cpp
vector<Student>::iterator lowerBoundByRoll(int roll) {
    return lower_bound(students.begin(), students.end(), roll,
                       [](const Student& s, int r) { return s.rollNo < r; });
}

vector<Student>::iterator findStudent(int roll) {
    auto pos = lowerBoundByRoll(roll);
    if (pos != students.end() && pos->rollNo == roll) return pos;
    return students.end();
}
```

`lower_bound()` returns the first element whose roll number is **not less than** the one given. That position is either the student we are looking for or just the place where it would be inserted. This is why `findStudent()` must check `pos->rollNo == roll` before treating it as a match.

The same `lowerBoundByRoll()` function gives the insertion point, and `insertSorted()` refuses to insert if `findStudent()` already finds that roll number. This guarantees roll numbers stay unique.

No comparison operator is needed in the struct, because the lambda supplies the comparison.

### Adding a student

```text
Enter name
    │
Enter roll number (must be unique)
    │
Enter number of subjects (1-20)
    │
For each subject: enter name (no duplicates), then marks (0-100)
    │
calculateGrade()  →  insertSorted()  →  unsaved = true
```

### Saving

Records are saved when:

- the user chooses **Save to File** (option 4), or
- the program ends (option 5, or the input stream is closed with Ctrl+D / Ctrl+Z) and there are unsaved changes.

If the file cannot be written (for example, the folder is read-only), an error message is shown. When this happens at exit, the unsaved changes are lost.

### Input handling

All input is read as a whole line with `getline()`, trimmed, and checked. If the input is invalid, the same question is asked again. When the input stream ends, `readLine()` throws an `InputClosed` exception, which `main()` catches before saving and exiting cleanly.

---

## 7. Input Rules

| Input | Rule |
|-------|------|
| Menu choice | Whole number from 1 to 5 |
| Name | Not empty, must not contain `,` or `:` (spaces are allowed) |
| Roll number | Whole number from 1 to 999999, not already used |
| Number of subjects | Whole number from 1 to 20 |
| Subject name | Not empty, must not contain `,` or `:`, spaces allowed, must not repeat for the same student (compared ignoring case, so `Maths` and `maths` count as the same subject) |
| Marks | Whole number from 0 to 100 |
| Search roll number | Whole number from 1 to 999999 |

Leading and trailing spaces are removed from every input. Commas and colons are rejected because the file format uses them as separators.

---

## 8. Grading

`calculateGrade()` adds up all marks and divides by the number of subjects:

```cpp
s.percentage = total / static_cast<float>(s.marks.size());
s.grade = gradeFor(s.percentage);
```

| Percentage | Grade |
|-----------|-------|
| 90 or more | A |
| 80 or more | B |
| 70 or more | C |
| 60 or more | D |
| below 60 | F |

Example:

```text
Chemistry = 89
Maths     = 98
Physics   = 84

Total      = 271
Subjects   = 3
Percentage = 271 / 3 = 90.33   →   Grade A
```

Percentages are displayed and saved with two decimal places. The grade is chosen from the unrounded value. `calculateGrade()` also guards against a student with no subjects (percentage 0, grade F), so it can never divide by zero. This cannot happen through the menu, but it keeps the function safe on its own.

---

## 9. File Format and Loading

### Format

One student per line:

```text
name,rollNo,percentage,grade,subject:marks,subject:marks,...
```

Example `students.txt`:

```text
Arjun,100,90.33,A,Chemistry:89,Maths:98,Physics:84
Ravi Kumar,150,82.50,B,Computer Science:95,Maths:70
Sweita,201,80.67,B,bio:86,chemistry:91,physics:65
```

### How loading works

`loadFromFile()` reads the file line by line. Each line is trimmed (so Windows line endings are handled) and passed to `parseStudent()`, which splits it with `stringstream` and `getline(..., ',')`.

- The **percentage and grade stored in the file are ignored**. They are recalculated from the marks, so editing the file by hand can never produce a wrong grade.
- Blank lines are ignored.
- A line is **skipped** (and counted) if:
  - it has fewer than four fields before the marks, or an empty name
  - the roll number is not a whole number from 1 to 999999
  - a subject entry has no `:`, an empty subject name, a repeated subject, or marks that are not a whole number from 0 to 100
  - it contains no subjects at all
  - its roll number already exists in an earlier line
- If the file does not exist, the program prints `No saved data found. Starting with an empty list.`
- After loading, the program reports how many records were loaded and how many lines were skipped. Skipped lines are removed from the file the next time it is saved.

---

## 10. Function Reference

| Function | Purpose |
|----------|---------|
| `trim` | Removes leading and trailing whitespace |
| `toLower` | Lowercases a string (used for case-insensitive subject comparison) |
| `parseInt` | Converts a whole string to `int`; returns `false` if it is not a valid number |
| `readLine` | Prints a prompt and reads one trimmed line; throws `InputClosed` at end of input |
| `readInt` | Asks until the user enters a whole number in a given range |
| `readField` | Asks until the user enters non-empty text without `,` or `:` |
| `gradeFor` | Converts a percentage to a grade letter |
| `calculateGrade` | Calculates a student's percentage and grade |
| `hasSubject` | Checks if a student already has a subject (ignoring case) |
| `lowerBoundByRoll` | Binary search for the first student whose roll number is not less than the given one |
| `findStudent` | Returns the student with a roll number, or `students.end()` |
| `insertSorted` | Inserts a student at the correct position; `false` if the roll number exists |
| `readRollNo` | Asks for a roll number until it is valid and unused |
| `readSubject` | Reads one subject name and its marks |
| `addStudent` | Reads a full student, calculates the grade, inserts, and sets `unsaved` |
| `nameColumnWidth` | Computes a name column wide enough for the longest name |
| `displayStudents` | Prints all students in a formatted table |
| `searchStudent` | Finds a student by roll number and prints the result |
| `saveToFile` | Writes all records to `students.txt`; returns `false` on failure |
| `parseSubjectEntry` | Parses one `subject:marks` entry |
| `parseStudent` | Parses one line of the file into a `Student` |
| `loadFromFile` | Loads all valid records from `students.txt` |
| `printMenu` | Prints the menu |
| `runMenu` | Runs the menu loop until the user exits |
| `main` | Loads data, runs the menu, saves unsaved changes on exit |

---

## 11. Complexity

Let `n` be the number of students and `m` the number of subjects per student (at most 20).

| Operation | Time |
|-----------|------|
| Search by roll number | O(log n) |
| Add student | O(m² + n) |
| Display all | O(n × m) |
| Save to file | O(n × m) |
| Load from file | O(n × m²) for a sorted file, up to O(n²) if the file is not sorted |

Notes:

- Search is O(log n) because the vector is always sorted and uses `lower_bound()`.
- Adding costs O(n) because inserting into the middle of a vector shifts the elements after it. Checking for repeated subjects scans the subjects entered so far, which gives the m² term. Since `m ≤ 20`, that part is tiny.
- Loading a file that was saved by this program is fast, because the records are already in roll-number order and are inserted at the end.
- Space is O(n × m), since every student stores all of their subjects.

---

## 12. Limitations

- The percentage is a simple average and assumes every subject is out of 100.
- There is no option to delete or edit a student after it has been added.
- Names and subject names cannot contain `,` or `:`.
- A student can have at most 20 subjects, and roll numbers must be between 1 and 999999.
- Search works by roll number only.
- Records are stored in a plain text file that is fully rewritten on every save, and the program is meant for a single user.
- Lines that cannot be loaded are dropped on the next save.
- If the file cannot be written when the program exits, unsaved changes are lost (an error message is shown).

---

## 13. Possible Improvements

- Delete and update a student
- Search by name, and sort or filter by percentage or grade
- A configurable maximum mark per subject
- A more robust storage format (quoted CSV or JSON), which would also allow commas in names
- Splitting the code into header and source files
- Automated tests for the parsing, grading and search functions

---

## 14. Concepts Demonstrated

- **Data structures:** `struct`, `vector`, `map`
- **Algorithms:** binary search with `lower_bound()`, sorted insertion
- **C++ features:** lambdas, `const auto&` loops, default member initializers, exceptions (`try` / `catch`), `static_cast`
- **Strings and streams:** `getline`, `stringstream`, `stoi`, `find`, `substr`, `transform`
- **File handling:** `ifstream`, `ofstream`, parsing and validating text data
- **Formatted output:** `setw`, `left`, `fixed`, `setprecision`
- **Defensive programming:** input validation, safe parsing, error checks on file writes
