#include <sys/stat.h>
#include <limits>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <unordered_map>
using namespace std;

// ------------ FUNCTION PROTOTYPES BEGIN ------------ //

void clearScreen();
vector<string> splitS (const string &s, const string &delimiter);
inline bool fileExists (const string &name);
void readString (string &input);
string makeNewData (int &choice);
string xorCipher(const string &str, const string &password);
string hashString (const string& str);

// ------------ ^FUNCTION PROTOTYPES END^ ------------- //


// ------------ CLASSES BEGIN ------------ //

class Personne {
    public:
        int id;
        string nom;
        int zoneGeo;
        string formatForFile() {return to_string(id) + " " + nom + " " + to_string(zoneGeo);}
        string formatForUser(const string &temp) {return temp + "\nid:" + to_string(id) + " name:" + nom + " zone:" + to_string(zoneGeo);}
};

class Recommande {
    public:
        int id;
        string type;
        int idFacteur;
        int idDestinataire;
        Recommande(int x, string s, int f, int d) {id = x; type = s; idFacteur = f; idDestinataire = d;}
        string formatForFile() {return to_string(id) + " " + type + " " + to_string(idFacteur) + " " + to_string(idDestinataire);}
        string formatForUser() {return "RECOMMANDE\nid:" + to_string(id) + " type:" + type + " idFacteur:" + to_string(idFacteur) + " idDestinataire:" + to_string(idDestinataire);}
};

class Facteur : public Personne {
    public:
        Facteur(int x, string n, int z) {id = x; nom = n; zoneGeo = z;}
        
};

class Habitant : public Personne {
    public:
        Habitant(int x, string n, int z) {id = x; nom = n; zoneGeo = z;}
};

// ------------ ^CLASSES END^ ------------ //


// WARNING !!!!!!!!! SPAGHETTI CODE AHEAD !!!!!!!!! WARNING //
int main() {
    string filename, line, password;
    int deleteLine, choice;

    clearScreen();

    while (true) {

        cout << "Give database name:" << endl;
        readString(filename);
        clearScreen();
        filename = filename + ".edat";

        if (fileExists(filename)) {

            cout << "Database already exists, give password:" << endl;
            readString(password);
            clearScreen();

            ifstream fileIn;
            fileIn.open(filename);
            getline(fileIn, line);
            line = xorCipher(line,password);

            if (hashString(password) == line) {
                cout << "password correct" << endl;
                break;
            } else {
                clearScreen();
                cout << "password incorrect...\n" << endl;
            }

        } else {

            cout << "Creating new database...\nPassword required:" << endl;
            readString(password);

            ofstream fileOut; //create new file

            fileOut.open(filename, ios_base::app);
            fileOut << xorCipher(hashString(password),password) << endl;

            fileOut.close();

            break;
        }

    }

    while (true) {

        clearScreen();
        cout << "Choose action (0:Add line, 1:delete line, 2:read):" << endl;
        cin >> choice;
        clearScreen();

        if (choice == 0) { // -------------- ADDING A NEW LINE -------------- //

            int classType;
            string output;

            cout << "What to add? (0:Facteur, 1:Habitant, 2:Recommande):" << endl;
            cin >> choice;
            classType = choice;

            clearScreen();
            output = xorCipher(to_string(classType) + " " + makeNewData(choice),password);

            ofstream fileOut; //create or open file

            fileOut.open(filename, ios_base::app);
            fileOut << output << endl;

            fileOut.close();

        } else if (choice == 1) { // ------------ REMOVING GIVEN LINE ------------ //
            int i=0;

            cout << "Give line number to delete:" << endl;
            cin >> deleteLine;

            ifstream fileIn;
            fileIn.open(filename);
            ofstream temp;
            temp.open(".tmp.tedat");

            while (getline(fileIn, line)) {
                // write all lines to temp other than the line marked for erasing
                if (i != deleteLine || i == 0)
                    temp << line << std::endl;
                i++;
            }
            temp.close();
            fileIn.close();

            // Replace original file with the temp one
            const char * p = filename.c_str();
            remove(p);
            std::rename(".tmp.tedat", p);

        } else if (choice == 2) { // -------------- READING FILE -------------- //
            int i=0;

            ifstream fileIn;
            fileIn.open(filename);

            while (getline(fileIn, line)) {
                if (i==0) {i++; continue;}

                line = xorCipher(line,password);

                vector<string> vector_input;
                string toShow;
                int classType;

                vector_input = splitS(line, " ");
                classType = stoi(vector_input[0]);

                if (classType == 0) {

                    Facteur fac(stoi(vector_input[1]),vector_input[2],stoi(vector_input[3]));
                    toShow = fac.formatForUser("FACTEUR");

                } else if (classType == 1) {

                    Habitant hab(stoi(vector_input[1]),vector_input[2],stoi(vector_input[3]));
                    toShow = hab.formatForUser("HABITANT");

                } else if (classType == 2) {

                    Recommande reco(stoi(vector_input[1]),vector_input[2],stoi(vector_input[3]),stoi(vector_input[4]));
                    toShow = reco.formatForUser();
                }
                cout << to_string(i) + ":\n" + toShow + "\n" << endl;
                i++;
            }

            fileIn.close();
        }
        
        char tmp='0';
        cout << "Quit? (Y/N)" << endl;
        while (tmp != 'n' && tmp != 'N' && tmp != 'y' && tmp != 'Y') {
            cin >> tmp;
        }
        if (tmp == 'y' || tmp == 'Y') break;
        clearScreen();

    }

    clearScreen();
    return 0;
}

// -------------- FUNCTIONS BELOW -------------- //

/*
ifile.clear();
ifile.seekg(0);
*/


// Split a string into different strings using delimiter
vector<string> splitS(const string &s, const string &delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

// Check if a file exists
inline bool fileExists (const string &name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

// Clear terminal screen
void clearScreen() {
    #if defined _WIN32
        system("cls");
        //clrscr(); // including header file : conio.h
    #elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
        //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
    #elif defined (__APPLE__)
        system("clear");
    #endif
}

// SOMETHING THAT ACTUALLY READDS A DAMLN LIENE IRHOTUT BREAKING VEREYYTHIGN
// I KNOW THIS DOESN'T MAKE SENSE, BUT DO NOT TOUCH
void readString(string &input) {
    getline(cin, input);
    if (input == "") {
        readString(input);
    }
}


// Function to generate new line that will be added to file
string makeNewData(int &choice) {

    string text_input;
    vector<string> vector_input;

    if (choice == 0 || choice == 1) {

        Personne personne;
        cout << "Give <id> <name> <zone geo>:" << endl;
        readString(text_input);
        clearScreen();
        vector_input = splitS(text_input, " ");

        if (choice == 0) {
            personne = Facteur(stoi(vector_input[0]),vector_input[1],stoi(vector_input[2]));
            cout << "Successfully added:\n\n" + personne.formatForUser("FACTEUR") + "\n" << endl;
        }
        if (choice == 1) {
            personne = Habitant(stoi(vector_input[0]),vector_input[1],stoi(vector_input[2]));
            cout << "Successfully added:\n\n" + personne.formatForUser("HABITANT") + "\n" << endl;
        }

        return personne.formatForFile();

    } else if (choice == 2) {

        string type[2] = {"lettre", "colis"};
        cout << "type? (0:lettre, 1:colis)" << endl;
        cin >> choice;
        clearScreen();
        cout << "Give <id> <idFacteur> <idDestinataire>" << endl;
        readString(text_input);
        clearScreen();
        vector_input = splitS(text_input, " ");

        Recommande reco(stoi(vector_input[0]),type[choice],stoi(vector_input[1]),stoi(vector_input[2]));

        cout << "Successfully added:\n\n" + reco.formatForUser() + "\n" << endl;

        return reco.formatForFile();
    }

    return 0;
}

// Encrypt/Decrypt string using XOR Cipher
string xorCipher(const string &str, const string &password)
{
    string encryptedString;
    for (int i = 0; i < str.length(); i++)
    {
        encryptedString += (char)(str[i] ^ password[i % password.length()]);
    }
    return encryptedString;
}

// Homemade function to hash a string
string hashString(const string& str)
{
    size_t hash = 0;
    for (char c : str)
    {
        hash = hash * 31 + c;
    }
    return to_string(hash);
}