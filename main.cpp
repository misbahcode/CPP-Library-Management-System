#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

struct Book {
    char number[6];
    char name[50];
    char author[20];
};

struct Student {
    char admissionNumber[6];
    char name[20];
    char bookNumber[6];
    int token;
};

static_assert(std::is_trivially_copyable<Book>::value, "Book records must be binary-copyable");
static_assert(std::is_trivially_copyable<Student>::value, "Student records must be binary-copyable");

template <typename T>
std::vector<T> readRecords(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    std::vector<T> records;
    T record{};
    while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        records.push_back(record);
    }
    return records;
}

template <typename T>
bool writeRecords(const std::string& fileName, const std::vector<T>& records) {
    std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cout << "Could not open " << fileName << " for writing.\n";
        return false;
    }
    for (const T& record : records) {
        file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    }
    if (!file) {
        std::cout << "Error while writing " << fileName << ".\n";
        return false;
    }
    return true;
}

std::string readText(const std::string& prompt, std::size_t maxLength) {
    std::string value;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, value)) {
            std::cout << "\nInput ended. Exiting.\n";
            std::exit(0);
        }
        if (!value.empty() && value.size() <= maxLength) {
            return value;
        }
        std::cout << "Enter between 1 and " << maxLength << " characters.\n";
    }
}

int readInteger(const std::string& prompt) {
    while (true) {
        const std::string value = readText(prompt, 20);
        try {
            std::size_t parsed = 0;
            const int number = std::stoi(value, &parsed);
            if (parsed == value.size()) {
                return number;
            }
        } catch (const std::exception&) {
        }
        std::cout << "Enter a valid whole number.\n";
    }
}

void pause() {
    std::cout << "\nPress Enter to continue...";
    std::string ignored;
    std::getline(std::cin, ignored);
}

void copyText(char* destination, std::size_t capacity, const std::string& source) {
    std::strncpy(destination, source.c_str(), capacity - 1);
    destination[capacity - 1] = '\0';
}

std::string normalized(const char* value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

void showBook(const Book& book) {
    std::cout << "\nBook Number: " << book.number
              << "\nBook Name: " << book.name
              << "\nBook Author Name: " << book.author << '\n';
}

void showStudent(const Student& student) {
    std::cout << "\nAdmission Number: " << student.admissionNumber
              << "\nStudent Name: " << student.name
              << "\nBooks Issued: " << student.token;
    if (student.token == 1) {
        std::cout << "\nBook Number: " << student.bookNumber;
    }
    std::cout << '\n';
}

void addBooks() {
    std::vector<Book> books = readRecords<Book>("book.dat");
    char again = 'y';
    while (again == 'y' || again == 'Y') {
        Book book{};
        copyText(book.number, sizeof(book.number), readText("Enter book number (max 5): ", 5));
        copyText(book.name, sizeof(book.name), readText("Enter book name (max 49): ", 49));
        copyText(book.author, sizeof(book.author), readText("Enter author name (max 19): ", 19));
        books.push_back(book);
        std::cout << "Book created.\nAdd another book? (y/n): ";
        std::string answer;
        std::getline(std::cin, answer);
        again = answer.empty() ? 'n' : answer[0];
    }
    writeRecords("book.dat", books);
}

void addStudents() {
    std::vector<Student> students = readRecords<Student>("student.dat");
    char again = 'y';
    while (again == 'y' || again == 'Y') {
        Student student{};
        copyText(student.admissionNumber, sizeof(student.admissionNumber),
                 readText("Enter admission number (max 5): ", 5));
        copyText(student.name, sizeof(student.name), readText("Enter student name (max 19): ", 19));
        student.token = 0;
        student.bookNumber[0] = '\0';
        students.push_back(student);
        std::cout << "Student record created.\nAdd another student? (y/n): ";
        std::string answer;
        std::getline(std::cin, answer);
        again = answer.empty() ? 'n' : answer[0];
    }
    writeRecords("student.dat", students);
}

void displayBook(const std::string& number) {
    const auto books = readRecords<Book>("book.dat");
    const auto found = std::find_if(books.begin(), books.end(), [&](const Book& book) {
        return normalized(book.number) == normalized(number.c_str());
    });
    if (found == books.end()) {
        std::cout << "Book does not exist.\n";
    } else {
        showBook(*found);
    }
    pause();
}

void displayStudent(const std::string& admissionNumber) {
    const auto students = readRecords<Student>("student.dat");
    const auto found = std::find_if(students.begin(), students.end(), [&](const Student& student) {
        return normalized(student.admissionNumber) == normalized(admissionNumber.c_str());
    });
    if (found == students.end()) {
        std::cout << "Student does not exist.\n";
    } else {
        showStudent(*found);
    }
    pause();
}

void modifyBook() {
    auto books = readRecords<Book>("book.dat");
    const std::string number = readText("Enter book number: ", 5);
    const auto found = std::find_if(books.begin(), books.end(), [&](const Book& book) {
        return normalized(book.number) == normalized(number.c_str());
    });
    if (found == books.end()) {
        std::cout << "Record not found.\n";
    } else {
        showBook(*found);
        copyText(found->name, sizeof(found->name), readText("New book name: ", 49));
        copyText(found->author, sizeof(found->author), readText("New author name: ", 19));
        if (writeRecords("book.dat", books)) {
            std::cout << "Record updated.\n";
        }
    }
    pause();
}

void modifyStudent() {
    auto students = readRecords<Student>("student.dat");
    const std::string number = readText("Enter admission number: ", 5);
    const auto found = std::find_if(students.begin(), students.end(), [&](const Student& student) {
        return normalized(student.admissionNumber) == normalized(number.c_str());
    });
    if (found == students.end()) {
        std::cout << "Record not found.\n";
    } else {
        showStudent(*found);
        copyText(found->name, sizeof(found->name), readText("New student name: ", 19));
        if (writeRecords("student.dat", students)) {
            std::cout << "Record updated.\n";
        }
    }
    pause();
}

void deleteStudent() {
    auto students = readRecords<Student>("student.dat");
    const std::string number = readText("Enter admission number: ", 5);
    const auto oldSize = students.size();
    students.erase(std::remove_if(students.begin(), students.end(), [&](const Student& student) {
        return normalized(student.admissionNumber) == normalized(number.c_str());
    }), students.end());
    if (students.size() == oldSize) {
        std::cout << "Record not found.\n";
    } else if (writeRecords("student.dat", students)) {
        std::cout << "Student record deleted.\n";
    }
    pause();
}

void deleteBook() {
    auto books = readRecords<Book>("book.dat");
    const std::string number = readText("Enter book number: ", 5);
    const auto oldSize = books.size();
    books.erase(std::remove_if(books.begin(), books.end(), [&](const Book& book) {
        return normalized(book.number) == normalized(number.c_str());
    }), books.end());
    if (books.size() == oldSize) {
        std::cout << "Record not found.\n";
    } else if (writeRecords("book.dat", books)) {
        std::cout << "Book record deleted.\n";
    }
    pause();
}

void displayAllStudents() {
    const auto students = readRecords<Student>("student.dat");
    std::cout << "\nAdmission No.  Student Name       Books Issued\n"
              << "------------------------------------------------\n";
    for (const Student& student : students) {
        std::cout << student.admissionNumber << "\t\t" << student.name
                  << "\t\t" << student.token << '\n';
    }
    pause();
}

void displayAllBooks() {
    const auto books = readRecords<Book>("book.dat");
    std::cout << "\nBook No.  Book Name                       Author\n"
              << "------------------------------------------------------------\n";
    for (const Book& book : books) {
        std::cout << book.number << "\t" << book.name << "\t\t" << book.author << '\n';
    }
    pause();
}

void issueBook() {
    auto students = readRecords<Student>("student.dat");
    const std::string admissionNumber = readText("Enter student admission number: ", 5);
    const auto student = std::find_if(students.begin(), students.end(), [&](const Student& item) {
        return normalized(item.admissionNumber) == normalized(admissionNumber.c_str());
    });
    if (student == students.end()) {
        std::cout << "Student record does not exist.\n";
        pause();
        return;
    }
    if (student->token != 0) {
        std::cout << "The student must return the currently issued book first.\n";
        pause();
        return;
    }

    const auto books = readRecords<Book>("book.dat");
    const std::string bookNumber = readText("Enter book number: ", 5);
    const auto book = std::find_if(books.begin(), books.end(), [&](const Book& item) {
        return normalized(item.number) == normalized(bookNumber.c_str());
    });
    if (book == books.end()) {
        std::cout << "Book does not exist.\n";
    } else {
        student->token = 1;
        copyText(student->bookNumber, sizeof(student->bookNumber), book->number);
        if (writeRecords("student.dat", students)) {
            std::cout << "Book issued successfully. Return it within 15 days to avoid a fine.\n";
        }
    }
    pause();
}

void depositBook() {
    auto students = readRecords<Student>("student.dat");
    const std::string admissionNumber = readText("Enter student admission number: ", 5);
    const auto student = std::find_if(students.begin(), students.end(), [&](const Student& item) {
        return normalized(item.admissionNumber) == normalized(admissionNumber.c_str());
    });
    if (student == students.end()) {
        std::cout << "Student record does not exist.\n";
        pause();
        return;
    }
    if (student->token != 1) {
        std::cout << "This student has no book to return.\n";
        pause();
        return;
    }

    const auto books = readRecords<Book>("book.dat");
    const auto book = std::find_if(books.begin(), books.end(), [&](const Book& item) {
        return normalized(item.number) == normalized(student->bookNumber);
    });
    if (book != books.end()) {
        showBook(*book);
    } else {
        std::cout << "Issued book record is missing; clearing the student's issue record.\n";
    }
    int days = readInteger("How many days was the book kept? ");
    while (days < 0) {
        std::cout << "Days cannot be negative.\n";
        days = readInteger("How many days was the book kept? ");
    }
    if (days > 15) {
        std::cout << "Fine = Rs. " << (days - 15) * 15 << '\n';
    }
    student->token = 0;
    student->bookNumber[0] = '\0';
    if (writeRecords("student.dat", students)) {
        std::cout << "Book deposited successfully.\n";
    }
    pause();
}

void administratorMenu() {
    while (true) {
        std::cout << "\nADMINISTRATOR MENU\n"
                  << "1. Create student record\n2. Display all students\n"
                  << "3. Display a student\n4. Modify student\n5. Delete student\n"
                  << "6. Add books\n7. Display all books\n8. Display a book\n"
                  << "9. Modify book\n10. Delete book\n11. Back to main menu\n";
        const int choice = readInteger("Select an option: ");
        switch (choice) {
            case 1: addStudents(); break;
            case 2: displayAllStudents(); break;
            case 3: displayStudent(readText("Enter admission number: ", 5)); break;
            case 4: modifyStudent(); break;
            case 5: deleteStudent(); break;
            case 6: addBooks(); break;
            case 7: displayAllBooks(); break;
            case 8: displayBook(readText("Enter book number: ", 5)); break;
            case 9: modifyBook(); break;
            case 10: deleteBook(); break;
            case 11: return;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

int main() {
    std::cout << "LIBRARY MANAGEMENT SYSTEM\nBy: Misbah\n";
    while (true) {
        std::cout << "\nMAIN MENU\n1. Book issue\n2. Book deposit\n"
                  << "3. Administrator menu\n4. Exit\n";
        switch (readInteger("Select an option: ")) {
            case 1: issueBook(); break;
            case 2: depositBook(); break;
            case 3: administratorMenu(); break;
            case 4: return 0;
            default: std::cout << "Invalid choice.\n";
        }
    }
}
