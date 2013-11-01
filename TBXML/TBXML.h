// ================================================================================================
//  TBXML.h
//  Fast processing of XML files
//
// ================================================================================================
//  Created by Tom Bradley on 21/10/2009.
//  Version 1.5
//  
//  Copyright 2012 71Squared All rights reserved.b
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
// ================================================================================================
//  Modified: duc.truong@titancorp.vn

#ifndef _TBXML_H_
#define _TBXML_H_

#include <string>
using namespace std;

// ================================================================================================
//  Error Codes
// ================================================================================================
enum TBXMLErrorCodes {
    D_TBXML_SUCCESS = 0,

    D_TBXML_DATA_NIL,
    D_TBXML_DECODE_FAILURE,
    D_TBXML_MEMORY_ALLOC_FAILURE,
    D_TBXML_FILE_NOT_FOUND_IN_BUNDLE,
    
    D_TBXML_ELEMENT_IS_NIL,
    D_TBXML_ELEMENT_NAME_IS_NIL,
    D_TBXML_ELEMENT_NOT_FOUND,
    D_TBXML_ELEMENT_TEXT_IS_NIL,
    D_TBXML_ATTRIBUTE_IS_NIL,
    D_TBXML_ATTRIBUTE_NAME_IS_NIL,
    D_TBXML_ATTRIBUTE_NOT_FOUND,
    D_TBXML_PARAM_NAME_IS_NIL
};


// ================================================================================================
//  Defines
// ================================================================================================
#define D_TBXML_DOMAIN "com.71squared.tbxml"

#define MAX_ELEMENTS 100
#define MAX_ATTRIBUTES 100

#define TBXML_ATTRIBUTE_NAME_START 0
#define TBXML_ATTRIBUTE_NAME_END 1
#define TBXML_ATTRIBUTE_VALUE_START 2
#define TBXML_ATTRIBUTE_VALUE_END 3
#define TBXML_ATTRIBUTE_CDATA_END 4

// ================================================================================================
//  Structures
// ================================================================================================

/** The TBXMLAttribute structure holds information about a single XML attribute. The structure holds the attribute name, value and next sibling attribute. This structure allows us to create a linked list of attributes belonging to a specific element.
 */
typedef struct _TBXMLAttribute {
	char * name;
	char * value;
	struct _TBXMLAttribute * next;
} TBXMLAttribute;



/** The TBXMLElement structure holds information about a single XML element. The structure holds the element name & text along with pointers to the first attribute, parent element, first child element and first sibling element. Using this structure, we can create a linked list of TBXMLElements to map out an entire XML file.
 */
typedef struct _TBXMLElement {
	char * name;
	char * text;
	
	TBXMLAttribute * firstAttribute;
	
	struct _TBXMLElement * parentElement;
	
	struct _TBXMLElement * firstChild;
	struct _TBXMLElement * currentChild;
	
	struct _TBXMLElement * nextSibling;
	struct _TBXMLElement * previousSibling;
	
} TBXMLElement;

/** The TBXMLElementBuffer is a structure that holds a buffer of TBXMLElements. When the buffer of elements is used, an additional buffer is created and linked to the previous one. This allows for efficient memory allocation/deallocation elements.
 */
typedef struct _TBXMLElementBuffer {
	TBXMLElement * elements;
	struct _TBXMLElementBuffer * next;
	struct _TBXMLElementBuffer * previous;
} TBXMLElementBuffer;



/** The TBXMLAttributeBuffer is a structure that holds a buffer of TBXMLAttributes. When the buffer of attributes is used, an additional buffer is created and linked to the previous one. This allows for efficient memeory allocation/deallocation of attributes.
 */
typedef struct _TBXMLAttributeBuffer {
	TBXMLAttribute * attributes;
	struct _TBXMLAttributeBuffer * next;
	struct _TBXMLAttributeBuffer * previous;
} TBXMLAttributeBuffer;

class TBXML {
public:
	TBXML();
	~TBXML();

	TBXMLElement * rootXMLElement;

	bool initWithXMLString(std::string &aXMLString, std::string &error);
	bool initWithXMLFile(std::string &aXMLFile, std::string &error);

	static std::string elementName(TBXMLElement* aXMLElement);
	static std::string elementName(TBXMLElement* aXMLElement, std::string &error);
	static std::string textForElement(TBXMLElement* aXMLElement);
	static std::string textForElement(TBXMLElement* aXMLElement, std::string &error);
	static std::string valueOfAttributeNamed(std::string &aName, TBXMLElement* forElement);
	static std::string valueOfAttributeNamed(std::string &aName, TBXMLElement* forElement, std::string &error);

	static std::string attributeName(TBXMLAttribute* aXMLAttribute);
	static std::string attributeName(TBXMLAttribute* aXMLAttribute, std::string &error);
	static std::string attributeValue(TBXMLAttribute* aXMLAttribute);
	static std::string attributeValue(TBXMLAttribute* aXMLAttribute, std::string &error);

	static TBXMLElement* childElementNamed(std::string &aName, TBXMLElement* parentElement);
	static TBXMLElement* childElementNamed(std::string &aName, TBXMLElement* parentElement, std::string &error);
	static TBXMLElement* nextSiblingNamed(std::string &aName, TBXMLElement* searchFromElement);
	static TBXMLElement* nextSiblingNamed(std::string &aName, TBXMLElement* searchFromElement, std::string &error);

private:
	
	TBXMLElementBuffer * currentElementBuffer;
	TBXMLAttributeBuffer * currentAttributeBuffer;
	
	long currentElement;
	long currentAttribute;
	
	char* bytes;
	long bytesLength;

	static std::string errorWithCode(int code);
	void decodeBytes();
	int allocateBytesOfLength(long length, std::string &error);
	char* mallocateBytesOfLength(long length, std::string &error);
	TBXMLElement* nextAvailableElement();
	TBXMLAttribute* nextAvailableAttribute();
};

#endif	//_TBXML_H_