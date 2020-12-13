#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <math.h>
#include <time.h> 

using namespace std;

bool running = false; // determines if the program is currently displaying code, or has compiled and is now running code.

int topLine = 0; // this is the first line that will be shown at the top of the screen. 
int row = 0; // current selected row
int column = 0; // current selected column
const int linesShown = 34; // number of lines that are allowed  to be shown on the screen at one time
const int maxColumnsPerRow = 90; // max amount of characters in a line before text wrapping
vector<string> lines; // stores all lines as strings

string currentFileName = "main.cpp"; // default file name is main.cpp

void SetPosition(int X, int Y) // this method sets the console's cursor to a set position
{
	int interRows = 0; // rows that go onto a new line due to wrap

	for (auto l : lines) {
		int i = l.length() / maxColumnsPerRow;

		interRows += i;

		if (l == lines[row]) 
			break;		
	}

	X += (int)(log10((row-interRows)- topLine)) + 3; // This is to adjust for the line number display
	Y -= topLine;
	Y += interRows;


	HANDLE Screen;
	Screen = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Position = { X, Y };

	SetConsoleCursorPosition(Screen, Position);
}

void clear() { // clears all text in the console.
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}

void SetTextColor(int color) { // sets the current text colour.
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

vector<string> Types = { "int","double","using","bool","return","float","void", "char","namespace", "true", "false" }; // highlighted words

bool stringVectorContains(vector<string> vec, string value) { // determines if a string array contains a string value
	for (auto x : vec)
		if (x == value)
			return true;

	return false;
}

void UpdateLines() { // clears the console, formats and out puts line numbers and the text on each line
	clear();

	int highestDigits = log10(lines.size() + 1);

	int show = 0;
	if (lines.size() - topLine >= linesShown)
		show = linesShown;
	else
		show = lines.size() - topLine;

	int currentLine = topLine;
	for (auto i = lines.begin() + topLine; i != lines.begin() + topLine + show; ++i) {
		SetTextColor(4);
		int digits = log10(currentLine + 1);
		int nspaces = highestDigits - digits;
		string spaces = "";

		for (int n = 0; n < nspaces; n++)
			spaces += " ";

		cout << spaces << currentLine + 1 << "| ";

		bool lineColored = false;

		if ((*i).rfind("//", 0) == 0) {
			SetTextColor(10);
			lineColored = true;
		}
		else if ((*i).rfind("#", 0) == 0) {
			SetTextColor(96);
			lineColored = true;
		}
		else {
			SetTextColor(7);
		}
		
		int lineX = 0;
		if (lineColored)
			cout << *i << endl;
		else {
			vector<string> args;

			string arg = "";
			for (auto c = (*i).begin(); c != (*i).end(); ++c) {
				if (*c == ' ') {
					args.push_back(arg);
					arg = "";
				}
				else
					arg = arg + *c;
			}
			args.push_back(arg);

			bool isString = false;
			bool isComment = false;
			for (auto a : args) {
				bool over = false;
				if (a.rfind("\"", 0) == 0) {
					isString = true;
				}
				else if (a.rfind("\"", a.length() - 1) == 0) {
					SetTextColor(12);
					isString = false;
					over = true;
				}
				else if (stringVectorContains(Types, a)) {
					SetTextColor(9);
					over = true;
				}
				else if (a.find("//",0) == 0)
					isComment = true;

				if (isString)
					SetTextColor(12);
				else if (isComment)
					SetTextColor(10);
				else if (!over)
					SetTextColor(7);

				cout << a << " ";

				lineX++;
			}

			cout << endl;
		}

		currentLine++;
	}
}

int GetEndOfLine(int r) { // returns the end column of a row
	return lines[r].length();
}

void Backspace() { // logic for backspacing within a line.
	if (column > 0) { // check if there is text on the line
		if (column < lines[row].length()) // if the cursor is between text
			lines[row].erase(lines[row].begin() + column - 1, lines[row].begin() + column);
		else
			lines[row] = lines[row].substr(0, lines[row].length() - 1); // otherwise remove the last character

		column--; // back a column
	}
	else if (row > 0) { // check if there's another row above and we're deleting the current line
		lines.erase(lines.begin() + row); // destroy current line

		row--; // go up a row
		
		if (row < topLine) { // if we've backed up farther than the top line being "rendered"
			topLine--; // back up a line
		}

		column = GetEndOfLine(row); // get the end coloumn of the row we've backed up on
	}

	UpdateLines(); // redraw the lines

	SetPosition(column, row); // set new column and row postions
}

void Delete() { // for deleting characters with the delete key
	if (column < lines[row].length()) { // if we can delete between
		lines[row].erase(lines[row].begin() + column+1, lines[row].begin() + column+2);
	}

	UpdateLines(); // redraw
	SetPosition(column, row); // set cursor
}

string toStr(char c) { // casts a character into a std::string
	return string(1, c);
}

void EnterCharacter(string c) { // for typing characters on a line
	string str = lines[row];

	if (column < str.length()) // if we can insert inbetween characters
		str.insert(column, c);
	else
		str.append(c); // otherwise add to the end of the line

	lines[row] = str; // the line to the manipulated string
}

void SaveDocument() { // for saving the current document
	string name = "main.cpp"; // default name
	string toSave = ""; // text buffer

	for (auto i = lines.begin(); i != lines.end(); ++i) {
		string s = *i; // stores dereferenced line string
		toSave = toSave + s + "\n"; // append the line to the text buffer
			
		if (s.size() > 3 && s.substr(0,3) == "//@") { // check for special unique syntax
			vector<string> args; // stores words in the line

			string arg; // current word buffer
			for (auto x : s) {
				if (x == ' ') { // if there's a space we know the current word has ended
					args.push_back(arg); // add to args vector
					arg = ""; // reset buffer
				}
				else {
					arg = arg + x; // add character to buffer
				}
			}
			args.push_back(arg); // push final arg because lines tend to not end with a space

			if (args.size() > 1 && args[0] == "//@filename") { // if the user has specified the file's name
				name = args[1];
				currentFileName = name; // set current filename to this
			}
		}	
	}

	ofstream file(name); // open or make a new file 
	file << toSave; // write to file

	file.close(); // close out file
}

void LoadDocument() { // for loading files
	clear(); // clear the console

	cout << "Input the filename: \n"; // tell user to input filename

	string filename = ""; // stores input

	cin >> filename; // store input to filename

	if (filename != "") { // check if they actually input something
		ifstream file(filename); // look for file and open it

		string text; // text buffer

		lines.clear(); // clear current document

		while (getline(file, text)) { // read each line of the file
			lines.push_back(text); // add line
		}

		column = 0; // reset cursor
		row = 0;

		UpdateLines(); // redraw
		SetPosition(column, row); // set cursor
	}
}

void TypeCharacter(char ch) { // typing printable characters
	if (ch > 31 && ch < 128) { // check if the character is printable
		EnterCharacter(toStr(ch));

		column++; // add column 

		UpdateLines(); // redraw
		SetPosition(column, row); // set cursor
	}
}

void nonRunKeys() { // all key binds and shortcuts while not currently running a compiled cpp file
	int c = _getch();
	char ch;

	string command = "g++ " + currentFileName; // for some reason switch statements dislike it when you define a string like this inside a case

	if (c != 224) { // isn't arrow keys or delete/end/home
		switch (ch = c) {
		case 8: // backspace
			Backspace();
			break;
		case 9: // tab
			for (int i = 0; i < 3; i++) {
				EnterCharacter(" ");

				column++;
			}
			UpdateLines();
			SetPosition(column, row);
			break;
		case 19: // ctrl + s
			SaveDocument();
			break;
		case 15: // ctrl + o
			LoadDocument();
			break;
		case 63: // F5
			// compile! and run!
			system(command.c_str()); // user requires g++ and mingw be on their computer
			Sleep(1000);
			clear();
			system("a.exe"); // run program
			running = true;
			break;
		case 13: // enter, starts new line
			lines.insert(lines.begin() + row + 1, ""); 
			row++;
			column = 0;

			if (row >= linesShown + topLine) {
				topLine++;
			}

			UpdateLines();
			SetPosition(column, row);
			break;
		default:
			TypeCharacter(ch);
			break;
		}
	}
	else {
		switch (ch = _getch()) {
		case 72: // up key
			if (GetKeyState(VK_UP)) {
				if (row > 0) {
					row--;
					if (row < topLine) {
						topLine--;
					}

					if (column > lines[row].size()) {
						column = lines[row].size();
					}
				}

				UpdateLines();
				SetPosition(column, row);
				break;
			}
		case 80: // down key
			if (GetKeyState(VK_DOWN)) {
				if (row < lines.size() - 1) {
					row++;
					if (row > topLine + linesShown - 1) {
						topLine++;
					}

					if (column > lines[row].size()) {
						column = lines[row].size();
					}
				}

				UpdateLines();
				SetPosition(column, row);

				break;
			}
		case 75: // left key
			if (GetKeyState(VK_LEFT)) {
				if (column > 0) {
					column--;
				}
				else {
					if (row > 0) {
						row--;
						column = GetEndOfLine(row);
						if (row < topLine) {
							topLine--;
						}
					}
				}
				UpdateLines();
				SetPosition(column, row);
				break;
			}
		case 77: // right key
			if (GetKeyState(VK_RIGHT)) {
				if (column < lines[row].length()) {
					column++;
				}
				else {
					if (row != lines.size() - 1) {
						row++;
						column = 0;
						if (row > topLine + linesShown - 1) {
							topLine++;
						}
					}
				}
				UpdateLines();
				SetPosition(column, row);
				break;
			}
		case 83:
			Delete();
			break;
		case 79: // end key, goes to end of line
			column = lines[row].size();
			SetPosition(column, row);
			break;
		case 71: // home key, goes to start of line
			column = 0;
			SetPosition(column, row);
			break;
		default:
			break;
		}
	}
}

void RunKeys() { // keybinds while running a compiled cpp file
	char ch = _getch();

	switch (ch)
	{
	case 63: // f5, stop current program
		running = false;
		UpdateLines();
		SetPosition(column, row);
		break;
	default:
		break;
	}
}

int main()
{
	system("mode con: cols=90 lines=35"); // set console size
	SetConsoleTitle(L"Nya++ IDE"); // set titled

	vector<string> helloWorld = { // default program
	"#include <iostream>",
	"",
	"int main() {",
	"   std::cout << \"Hello World!\";",
	"   return 0;",
	"}"
	};

	for (auto & i : helloWorld) { // read lines and add it to the current lines
		lines.push_back(i); 
	}
	
	row = lines.size() - 1; // set row to last
	column = GetEndOfLine(row); // set column to end of the line

	UpdateLines(); // draw
	SetPosition(column, row); // set cursor

	while (1) {
		if (!running)
			nonRunKeys();
		else
			RunKeys();
	}
}