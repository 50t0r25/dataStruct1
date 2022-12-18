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

static const string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

// ------------ FUNCTION PROTOTYPES BEGIN ------------ //

void clearScreen();
vector<string> splitS (const string &s, const string &delimiter);
inline bool fileExists (const string &name);
void readString (string &input);
string makeNewData (int &choice);
string xorCipher(const bool &decrypt, string str, const string &password);
string hashString (const string& str);
static inline bool is_base64(unsigned char c);
vector<unsigned char> stringToUnsignedChar(const string& str);
string unsignedCharVectorToString(const vector<unsigned char>& vec);
string base64Encode(unsigned char const* buf, unsigned int bufLen);
vector<unsigned char> base64Decode(string const& encoded_string);
string stringToBase64(const string &in);
string base64ToString(const string &in);

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

    while (true) { // Loop to wait for correct database password

        cout << "Give database name:" << endl;
        readString(filename);
        clearScreen();
        filename = filename + ".edat";

        if (fileExists(filename)) { // File exists so ask for password

            cout << "Database already exists, give password:" << endl;
            readString(password);
            clearScreen();

            // Get the first line, which is an encrypted hash of the password
            ifstream fileIn;
            fileIn.open(filename);
            getline(fileIn, line);
            line = xorCipher(true, line,password);

            // Check if the password is correct
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

            // Create new database file
            // Then add an encrypted hash of the password in the first line of the file
            ofstream fileOut;
            fileOut.open(filename, ios_base::app);
            fileOut << xorCipher(false, hashString(password),password) << endl;

            fileOut.close();

            break;
        }

    }

    while (true) { // While loop for looping program and not closing when done

        clearScreen();
        cout << "Choose action (0:Add line, 1:delete data, 2:read):" << endl;
        cin >> choice;
        clearScreen();

        if (choice == 0) { // -------------- ADDING A NEW LINE -------------- //

            int classType;
            string output;

            cout << "What to add? (0:Facteur, 1:Habitant, 2:Recommande):" << endl;
            cin >> choice;

            // User is retarded
            if (choice != 0 && choice != 1 && choice != 2) {

                clearScreen();
                cout << "What?????????????????\n" << endl;
                return 1;

            }

            classType = choice;

            // Format then encrypt the user input into data to put in file
            clearScreen();
            output = xorCipher(false, to_string(classType) + " " + makeNewData(choice),password);

            //open file
            ofstream fileOut;
            fileOut.open(filename, ios_base::app);

            // Put data to file 
            fileOut << output << endl;

            fileOut.close();

        } else if (choice == 1) { // ------------ REMOVING GIVEN LINE ------------ //
            int i=0;
            bool successfullyDeleted = false;

            cout << "Give line number to delete:" << endl;
            cin >> deleteLine;

            ifstream fileIn;
            fileIn.open(filename);
            ofstream temp;
            temp.open(".tmp.tedat");

            while (getline(fileIn, line)) {
                // Write all lines to temp except the line marked for removing
                if (i != deleteLine || i == 0) {
                    temp << line << std::endl;
                } else successfullyDeleted = true;
                    
                i++;
            }
            temp.close();
            fileIn.close();

            // Replace original file with the temp one
            const char * p = filename.c_str();
            remove(p);
            std::rename(".tmp.tedat", p);

            clearScreen();
            if (successfullyDeleted) {
                cout << "Successfully deleted data point number: " + to_string(deleteLine) + "\n" << endl;
            } else cout << "Error: Line doesn't exist" << endl;
            

        } else if (choice == 2) { // -------------- READING FILE -------------- //
            int i=0;

            ifstream fileIn;
            fileIn.open(filename);

            // Loop through file line by line
            while (getline(fileIn, line)) {
                if (i==0) {i++; continue;}

                // Decrypt the current line
                line = xorCipher(true, line,password);

                vector<string> vector_input;
                string toShow;
                int classType;

                // Split the line into different variables then format for output
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

        } else { // User is actually retarded

            clearScreen();
            cout << "Apparently you can't read\n" << endl;
            return 1;

        }
        
        // Ask if user wants to Quit
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
// Im too tired to explain how this works, so just trust me bro
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

// Encrypt/Decrypt string using XOR Cipher while Encoding/Decoding from base64
// when the decrypt bool is true, the funtion decrypts, otherwise it encrypts
string xorCipher(const bool &decrypt, string str, const string &password) {

    if (decrypt) str = base64ToString(str);

    string encryptedString;
    for (int i = 0; i < str.length(); i++)
    {
        encryptedString += (char)(str[i] ^ password[i % password.length()]);
    }
    if (!decrypt) encryptedString = stringToBase64(encryptedString);
    
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

// Checks if a character is in base64
static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

// Converts a string to a vector of unsigned chars
vector<unsigned char> stringToUnsignedChar(const string& str)
{
    std::vector<unsigned char> result;
    for (char c : str)
    {
        result.push_back(static_cast<unsigned char>(c));
    }
    return result;
}

// Converts an unsigned char vector to string
string unsignedCharVectorToString(const vector<unsigned char>& vec)
{
    std::string str;
    for (auto c : vec)
    {
        str += c;
    }
    return str;
}

// Encodes a buffer of characters to base64
string base64Encode(unsigned char const* buf, unsigned int bufLen) {
  string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';
  }

  return ret;
}

// Decodes a base64 string to a vector of unsigned chars
vector<unsigned char> base64Decode(string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::vector<unsigned char> ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
          ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
  }

  return ret;
}

// Simplified function to encode string to base64
string stringToBase64(const string &in) {
    vector<unsigned char> temp = stringToUnsignedChar(in);
    return base64Encode(&temp[0], temp.size());
}

// Simplified function to decode string from base64
string base64ToString(const string &in) {
    return unsignedCharVectorToString(base64Decode(in));
}