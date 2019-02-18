// Declarations for Network and helper functions
//
// created:	25-8-2018
// version:	0.3
//
// last edit:	13-9-2018
//
//
/* definitions in:
 *
 * Network.cpp
 * Network_input.cpp
*/

#ifndef NETWORK_H
#define NETWORK_H

//=============================================================================
//	Headers

#include<string>
#include<ostream>
#include<fstream>
#include<stdexcept>

#include"Node.h"
#include"Element.h"
#include"Settings.h"

//=============================================================================
//
//	Network

/* a list of all nodes in the hydraulic network
 * container for nodes and elements
 * - allocates memory and stores new nodes and elements
 * - if list sizes are provided, node list is filled with
 *   pointers to 0 id nodes, while element list is filled
 *   with NULL pointers
 * - stored nodes and elements can't be overwritten (apart from dummies)
*/
class Network {
public:
	Settings settings;
		// symulation settings

	// constructors
	Network();
	~Network();

	// accessors
	int getNodeQty() const;
		// get quantity of nodes
	int getElementQty() const;
		// get quantity of elements
	Node*& getNode(const int&) const;
		// get a node pointer, input is id of node
	Element*& getElement(const int&) const;
		// get an element pointer, input is id of element

	// mutators
	void setNodeListSize(const int&);
		// set node list size
	void setElementListSize(const int&);
		// set element list size
	template<class Type>
	void storeNode(const int&);
		// generate and store node
		// - should be called
		//   Network::storeNode<const node_type>(idNumber)
		// - see Node.h for vald node_types
	
	// utility
	void discretize();
		// discretize network
	void eventHandler() const;
		// handle events
	std::ostream& log(std::ostream&) const;
		// log output
	
private:
	int nodeListSize;
		// number of nodes
	int elementListSize;
		// number of elements
	Node** nodeList;
		// list of nodes
	Element** elementList;
		// list of elements

	// generation
	void deleteNodeList();
		// free node list memory
	void deleteElementList();
		// free element list memory
	void deleteLinks() const;
		// delete node links
	int getAvailableElement() const;
		// get id of first non-full element
	void initializeNodeList();
		// fill with dummy nodes
	void initializeElementList();
		// fill with NULLs
	void link() const;
		// store element links for each node
	void assignNeighbours() const;
	
	// input
	void read();
		// generate network from input files
	void process(std::ifstream&, const std::string&, int&);
		// '{' marks an opening of a processing instance
		// '}' marks a closing of a processing instance
		// only 2 input instances can be open at the same time
	void processTag(std::ifstream&, const std::string&, const int&);
		// process input tag
	void extractFromLine(const std::string&, std::string&,
				std::string&, const std::string&);
		// extract tags from line
	void cleanUp(std::string&);
		// clean up string
	void handleElements(const std::string&, const std::string&, const int&);
		// handle element generation member setting
	void handleNodes(const std::string&, const std::string&, const int&);
		// handle node generation and member setting
	void handleSettings(const std::string&, const std::string&, const int&);
	void validate(const std::string&);
		// validate input
	void processComment(std::ifstream&);
};

//	Templates =============================================================

template<class Type>
void Network::storeNode(const int& id)
	// generates and stores node
{
	// check bounds
	if (id > nodeListSize || id < 1)
		throw std::runtime_error("Network::storeNode(): id");
	// check if node already exists
	if (getNode(id)->getType() != Node::NODE)
		throw std::runtime_error("Network::storeNode(): overwrite");

	// check which elements will need to be updated
	int* update = new int [elementListSize]();
	Element* e;

	for (int i {1}; i <= elementListSize; i++) {
		// assign
		e = this->getElement(i);

		if (e->getStart().getId() == id) {
			update[i-1] = 1;
		}
		else if (e->getEnd().getId() == id) {
			update[i-1] = -1;
		}
	}

	// delete old node
	delete getNode(id);
	// reassign
	getNode(id) = new Type {};

	// update elements
	for (int i {1}; i <= elementListSize; i++) {
		// assign
		e = this->getElement(i);

		if (update[i-1] == 1)
			e->setStart(getNode(id));
		else if (update[i-1] == -1)
			e->setEnd(getNode(id));
	}

	delete[] update;
}

#endif

