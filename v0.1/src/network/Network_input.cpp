// Definitions for Network input member functions
//
// created:	28-8-2018
// version:	0.2
//
// tested:	12-9-2018
// status:	working
//
// last edit: 	12-9-2018

//=============================================================================
//	
//	Headers

#include<stdexcept>
#include<cctype>
#include<fstream>
#include<string>

#include"Network.h"
using namespace std;

//=============================================================================
//
//	Declarations

string NODE_TYPE {};
	// node input mode
int LAST {0};
	// last stored node id

//=============================================================================
//
//	Input

//=============================================================================
//	Private

void Network::read()
{	
	string s;

	for (int i {0}; i < 3; i++) {

		if (i == 0) {
			s = "settings";
		}
		else if (i == 1) {
			s = "elements";
		}
		else {
			s = "nodes";
		}
		
		ifstream ifs {s};
		// check if file is present
		if (!ifs)
			throw runtime_error(s+" file missing");

		// set excaption mask
		ifs.exceptions(ifs.badbit);

		// set instance counter
		int open {0};

		// read input file
		process(ifs, s, open);

		// final validation
		validate(s);
	}
}

void Network::process(ifstream& ifs, const string& file, int& open)
	// '{' marks an opening of a processing instance
	// '}' marks a closing of a processing instance
	// only 2 input instances can be open at the same time
{
	char ch;

	// process file
	while (ifs) {
		ch = ifs.get();

		// handle possible comment
		if (ch == '/') {
			processComment(ifs);
		}
		// skip whitespaces
		else if (isspace(ch)) {
			continue;
		}
		// begin filling network
		else if (ch == '{') {
			// mark instance opening
			open++;
			// check for number of open instances
			if (open > 2)
				throw runtime_error("input::1");
				
			// process input instance
			process(ifs, file, open);
		}
		// validate before ending
		else if (ch == '}') {
			// mark instance closing
			open--;
			// check for number of closed instances
			if (open < 0)
				throw runtime_error("input::2");
			return;
		}
		// handle preprocess tag
		else if (isalpha(ch)) {
			// return to stream
			ifs.putback(ch);

			// process a tag and it's input
			processTag(ifs, file, open);
		}
	}
	// check if all instances finished
	if (open != 0)
		throw runtime_error("input::3");
}

void Network::processTag(ifstream& ifs, const string& file, const int& open)
{
	string line {""};
	string tag {""};
	string num {""};

	// get line
	getline(ifs, line);

	// extract
	extractFromLine(line, tag, num, file);

	// check where to store

	if (file == "elements") {
		handleElements(tag, num, open);
	}
	else if (file == "nodes"){
		handleNodes(tag, num, open);
	}
	else {
		handleSettings(tag, num, open);
	}
}

void Network::extractFromLine(const string& line, string& tag, string& num,
				const string& file)
{
	// find delimiters
	unsigned long pos_1 {line.find_first_of(':')};
	unsigned long pos_2 {line.find_first_of(';')};
	
	// extract
	if (pos_1 != string::npos && pos_2 != string::npos) {
		// extract
		tag = line.substr(0, pos_1);
		num = line.substr(pos_1+1, pos_2-pos_1-1);
		// clean
		cleanUp(tag);
		cleanUp(num);
	}
	else if (pos_1 == string::npos && pos_2 == string::npos
		&& file == "nodes") {
		// possible node_type tag
		for (int i {0}; isalpha(line[i]); i++) {
			tag += line[i];
		}
		// clean
		cleanUp(tag);
	}
	else
		throw runtime_error("input::4");
}

void Network::cleanUp(string& s)
{
	string temp {""};

	for (int i {0}; i < s.size(); i++) {
		if (isspace(s[i]))
			continue;
		else if (isupper(s[i]))
			temp += tolower(s[i]);
		else
			temp += s[i];
	}

	s = temp;
}

void Network::handleElements(const string& tag, const string& num, const int& open)
{
	// check for list tags
	if (tag == "nodeqty") {
		// no instances should be open here
		if (open != 0)
			throw runtime_error("input::5");

		// check for reassignment
		if (nodeListSize != 0)
			throw runtime_error("input::6");
		
		// set list size
		setNodeListSize(stoi(num));
		return;
	}
	else if (tag == "elementqty" && elementListSize == 0) {
		// no instances should be open here
		if (open != 0)
			throw runtime_error("input::7");

		// check for reassignment
		if (elementListSize != 0)
			throw runtime_error("input::8");

		// set list size
		setElementListSize(stoi(num));
		return;
	}

	// check if lists were initialized
	if (nodeListSize == 0 || elementListSize == 0)
		throw runtime_error("input::9");
	// two instances should be open here
	if (open != 2)
		throw runtime_error("input::10");

	// handle element generation/data storage
	int id {getAvailableElement()};

	// check if first is dummy
	if (getElement(id)->getId() == 0) {
		if (tag != "id")
			throw runtime_error("input::11");
		// check if tag matches
		if (id != stoi(num))
			throw runtime_error("input::12");
		// generate and store
		getElement(id)->setId(stoi(num));
	}
	// check for start/end tag
	else if (tag == "start") {
		// check if already present
		if (getElement(id)->hasStart())
			throw runtime_error("input::13");
		// store node id
		if (getNode(stoi(num))->getId() == 0)
			getNode(stoi(num))->setId(stoi(num));
		// store to element
		getElement(id)->setStart(getNode(stoi(num)));
	}
	else if (tag == "end") {
		// check if already present
		if (getElement(id)->hasEnd())
			throw runtime_error("input::14");
		// store node id
		if (getNode(stoi(num))->getId() == 0)
			getNode(stoi(num))->setId(stoi(num));
		// store to element
		getElement(id)->setEnd(getNode(stoi(num)));
	}
	// let element handle input
	else {
		handleInput(tag, num, getElement(id));
	}
}

void Network::handleNodes(const string& tag, const string& num, const int& open)
{
	// check for node_type tags
	if (num == "") {
		// no instances should be open here
		if (open != 0)
			throw runtime_error("input::15");

		// node_type check
		if (tag == "node") {
			NODE_TYPE = tag;
			return;
		}
		else if (tag == "source") {
			NODE_TYPE = tag;
			return;
		}
		else if (tag == "reservoir") {
			NODE_TYPE = tag;
			return;
		}
		else if (tag == "valve") {
			NODE_TYPE = tag;
			return;
		}
		else
			throw runtime_error("input::16");
	}
	
	// check if node_type is set and two instances are open
	if (open != 2 || NODE_TYPE == "")
		throw runtime_error("input::20");
	
	// check if tag is id
	if (tag == "id") {
		LAST = stoi(num);
		if (NODE_TYPE == "source") {
			// overrite dummy node
			storeNode<Source>(LAST);
		}
		else if (NODE_TYPE == "reservoir") {
			// overrite dummy node
			storeNode<Reservoir>(LAST);
		}
		else if (NODE_TYPE == "valve") {
			// overrite dummy node
			storeNode<Valve>(LAST);
		}
	}

	// let node handle input
	if (!nodeList[LAST-1]->handleInput(tag, num))
		throw runtime_error("Network::handleNodes(): invalid");
}

void Network::handleSettings(const string& tag, const string& num, const int& open)
{
	// one instance should be open here
	if (open != 1)
		throw runtime_error("input::22");

	// let settings handle input
	handleInput(tag, num, settings);
}

void Network::validate(const string& file)
{
	if (file == "elements") {
		// validate
		for (int j {0}; j < elementListSize; j++) {
			if (!elementList[j]->isFull())
				throw runtime_error("input::17");
		}
	}
	else if (file == "nodes") {
		Node* n;

		// check that there are at least two reservoirs in network
		int sources {0};
		// at least 1 pressure/head must be set
		int notSet {true};

		for (int i {1}; i <= nodeListSize; i++) {
			// assign
			n = this->getNode(i);

			// let node handle primary validation
			n->isValid();

			// compute head
			if (n->getPressure() != 0) {
				notSet = false;
				n->computeHead(settings.fluid);
			}
			// compute pressure
			else if (n->getHead() != 0) {
				notSet = false;
				n->computePressure(settings.fluid);
			}
				
			// sink/source count
			if (Reservoir* r =
			    down_cast<Reservoir>(n)) {
				sources++;
			}
			else if (Source* s =
				down_cast<Source>(n)) {
				sources++;

			}
		}
		if (sources < 2)
			throw runtime_error("input::18");
		else if (notSet)
			throw runtime_error("input::21");
	}
	// let settings validate
	else if (!settings.isValid()){
		throw runtime_error("input::23");
	}
}

void Network::processComment(ifstream& ifs)
{
	string s;

	getline(ifs, s);
	
	// check for comment
	if (s[0] == '/')
		return;
	throw runtime_error("input::19");

}

