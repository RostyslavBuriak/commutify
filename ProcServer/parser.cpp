#include "parser.h"

htmlparser::htmlparser(std::string _text) :text(_text) {

}


void htmlparser::DaneOgolneParsePL(Student& student) { //All needed data is located between '>' and '<' so when we parse string we have to substring it

	std::string word;
	std::istringstream sstream(text); //divide text by wrods
	while (sstream >> word) { //parse Wydzial
		if (word == u8"class=\"tabDwuCzesciowaLLeft\">Wydział</td><td") { //if we find a needed word
			sstream >> word;
			student.department += word.substr(word.find(">") +1) + " ";

			while (sstream >> word && word.find("<") == std::string::npos ) {
				student.department += word + " ";
			}

			student.department += word.substr(0, word.find("<"));
			break;
		}
	}
	while (sstream >> word) { //parse kierunek
		if (word == "class=\"tabDwuCzesciowaLLeft\">Kierunek</td><td") { //if we find a needed word
			sstream >> word;
			student.faculty += word.substr(word.find(">") + 1);

			while (sstream >> word && word.find("<") == std::string::npos) {
				student.faculty += word + " ";
			}

			student.faculty += word.substr(0, word.find("<"));
			break;
		}
	}
	while (sstream >> word) { //parse Specjalnosc
		if (word == u8"class='gridDaneHead'><td>Specjalność</td><td>Specjalizacja</td><td>Typ</td></tr><tr><td") { //if we find a needed word
			while (sstream >> word) {
				if (word.find(">") != std::string::npos) {
					student.specialization += word.substr(word.find(">")) + " ";

					while (sstream >> word && word.find("<") == std::string::npos) {
						student.specialization += word + " ";
					}

					student.specialization += word.substr(0, word.find("<"));
					break;
				}
			}
			break;
		}
	}
	while (sstream >> word) { //parse kierunek
		if (word == "class=\"tabDwuCzesciowaLLeft\">Semestr</td><td") { //if we find a needed word
			
			sstream >> word;
			student.semester += word.substr(word.find(">")+1,1);

			break;
		}
	}
}


void htmlparser::DaneOgolneParseEN(Student& student) { //All needed data is located between '>' and '<' so when we parse string we have to substring it

	std::string word;
	std::istringstream sstream(text); //divide text by wrods
	while (sstream >> word) { //parse name
		if (word.find("class=\"who_is_logged_in\">") != std::string::npos) { //if we find a needed word

			student.name = word.substr(word.find(">") + 1);

			while (sstream>>word)
			{
				if (word == "-") // '-' means the end of the name
					break;

				student.name += " " + word; //add next name past to student name
			}

			break;
		}
	}
	while (sstream >> word) { //parse Wydzial
		if (word == "class=\"tabDwuCzesciowaLLeft\">Department</td><td") { //if we find a needed word
			sstream >> word;
			student.department += word.substr(word.find(">") + 1) + " ";

			while (sstream >> word && word.find("<") == std::string::npos) {
				student.department += word + " ";
			}

			student.department += word.substr(0, word.find("<"));
			break;
		}
	}
	while (sstream >> word) { //parse kierunek
		if (word == "class=\"tabDwuCzesciowaLLeft\">Faculty</td><td") { //if we find a needed word
			sstream >> word;
			student.faculty += word.substr(word.find(">") + 1);

			while (sstream >> word && word.find("<") == std::string::npos) {
				student.faculty += word + " ";
			}

			student.faculty += word.substr(0, word.find("<"));
			break;
		}
	}
	while (sstream >> word) { //parse Specjalnosc
		if (word == "class='gridDaneHead'><td>Specialization</td><td></td><td></td></tr><tr><td") { //if we find a needed word
			while (sstream >> word) {
				if (word.find(">") != std::string::npos) {
					student.specialization += word.substr(word.find(">")+1) + " ";

					while (sstream >> word && word.find("<") == std::string::npos) {
						student.specialization += word + " ";
					}

					student.specialization += word.substr(0, word.find("<"));
					break;
				}
			}
			break;
		}
	}
	while (sstream >> word) { //parse semester
		if (word == "class=\"tabDwuCzesciowaLLeft\">Semester</td><td") { //if we find a needed word

			sstream >> word;
			student.semester += word.substr(word.find(">") + 1, 1);

			break;
		}
	}
}


std::string htmlparser::ViewDataParse(std::string value) {
	std::string word;
	std::istringstream sstream(text); //divide text by words
	while (sstream >> word) { //while there are still words
		if (word == value) { //if we find a needed word
			while (sstream >> word) { //starts from word which we found
				if (word.find("value")!= std::string::npos) { //our main goal is to find "value" 
					return word.substr(word.find("value") + 7, word.length()-1 - (word.find("value") + 7)); //if we find a "value" return value of this "value"
				}
			}
		}
	}
	return "";
}