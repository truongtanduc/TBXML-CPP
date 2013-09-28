// ================================================================================================
//  TBXML.m
//  Fast processing of XML files
//
// ================================================================================================
//  Created by Tom Bradley on 21/10/2009.
//  Version 1.5
//  
//  Copyright 2012 71Squared All rights reserved.
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
#include "TBXML.h"
#include <malloc.h>
#include <assert.h>
#include <fstream>
using namespace std;

// ================================================================================================
// Public Implementation
// ================================================================================================

TBXML::~TBXML() {
	if (bytes) {
		free(bytes);
		bytes = NULL;
	}

	while (currentElementBuffer) {
		if (currentElementBuffer->elements)
			free(currentElementBuffer->elements);

		if (currentElementBuffer->previous) {
			currentElementBuffer = currentElementBuffer->previous;
			free(currentElementBuffer->next);
		} else {
			free(currentElementBuffer);
			currentElementBuffer = 0;
		}
	}

	while (currentAttributeBuffer) {
		if (currentAttributeBuffer->attributes)
			free(currentAttributeBuffer->attributes);

		if (currentAttributeBuffer->previous) {
			currentAttributeBuffer = currentAttributeBuffer->previous;
			free(currentAttributeBuffer->next);
		} else {
			free(currentAttributeBuffer);
			currentAttributeBuffer = 0;
		}
	}
}

TBXML::TBXML() {
	rootXMLElement = NULL;

	currentElementBuffer = 0;
	currentAttributeBuffer = 0;

	currentElement = 0;
	currentAttribute = 0;

	bytes = 0;
	bytesLength = 0;
}

bool TBXML::initWithXMLString(std::string aXMLString, std::string error) {
	// allocate memory for byte array
	strcpy(bytes, aXMLString.c_str());

	// set null terminator at end of byte array
	bytesLength = aXMLString.length();
    bytes[bytesLength] = 0;

    // decode xml data
    this->decodeBytes();
    if (error.length() > 0) {
    	return false;
    }
	return true;
}

bool TBXML::initWithXMLFile(std::string aXMLFile, std::string error) {
	ifstream file (aXMLFile.c_str(), ios::in|ios::binary|ios::ate);
	if (file.is_open())
	{
	    file.seekg(0, ios::end);
	    int size = file.tellg();
	    bytes = new char [size+1];
	    file.seekg (0, ios::beg);
	    file.read (bytes, size);
	    file.close();

		return true;
	}
	else {
		bytes = NULL;
	    error.clear();
	    error.append(TBXML::errorWithCode(D_TBXML_FILE_NOT_FOUND_IN_BUNDLE));

	    return false;
	}
}

int TBXML::allocateBytesOfLength(long length, std::string error) {
    bytesLength = length;

    int rev = D_TBXML_SUCCESS;
    std::string localError = "";
    if(!length) {
        localError = TBXML::errorWithCode(D_TBXML_DATA_NIL);
    }

	bytes = (char*)malloc(bytesLength+1);

    if(!bytes) {
    	localError = TBXML::errorWithCode(D_TBXML_DATA_NIL);
    }

    error.clear();
    error.append(localError);

    return rev;
}

char* TBXML::mallocateBytesOfLength(long length, std::string error) {
	if (this->allocateBytesOfLength(length, error) == D_TBXML_SUCCESS) {
		return bytes;
	}
	return NULL;
}

std::string TBXML::elementName(TBXMLElement* aXMLElement) {
	if (NULL == aXMLElement->name) return "";
	std::string rev(aXMLElement->name);
	return rev;
}

std::string TBXML::elementName(TBXMLElement* aXMLElement, std::string error) {
    // check for nil element
    if (NULL == aXMLElement) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
        return "";
    }
    
    // check for nil element name
    if (NULL == aXMLElement->name || strlen(aXMLElement->name) == 0) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_NAME_IS_NIL));
        return "";
    }
    
	std::string rev(aXMLElement->name);
	return rev;
}

std::string TBXML::attributeName(TBXMLAttribute* aXMLAttribute) {
	if (NULL == aXMLAttribute->name) return "";
	std::string rev(aXMLAttribute->name);
	return rev;
}

std::string TBXML::attributeName(TBXMLAttribute* aXMLAttribute, std::string error) {
    // check for nil attribute
    if (NULL == aXMLAttribute) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ATTRIBUTE_IS_NIL));
        return "";
    }
    
    // check for nil attribute name
    if (NULL == aXMLAttribute->name) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ATTRIBUTE_NAME_IS_NIL));
        return "";
    }
    
	std::string rev(aXMLAttribute->name);
	return rev;
}


std::string TBXML::attributeValue(TBXMLAttribute* aXMLAttribute) {
	if (NULL == aXMLAttribute->value) return "";
	std::string rev(aXMLAttribute->value);
	return rev;
}

std::string TBXML::attributeValue(TBXMLAttribute* aXMLAttribute, std::string error) {
    // check for nil attribute
    if (NULL == aXMLAttribute) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ATTRIBUTE_IS_NIL));
        return "";
    }

	std::string rev(aXMLAttribute->value);
	return rev;
}

std::string TBXML::textForElement(TBXMLElement* aXMLElement) {
	if (NULL == aXMLElement->text) return "";

	std::string rev(aXMLElement->text);
	return rev;
}

std::string TBXML::textForElement(TBXMLElement* aXMLElement, std::string error) {
    // check for nil element
    if (NULL == aXMLElement) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
        return "";
    }
    
    // check for nil text value
    if (NULL == aXMLElement->text || strlen(aXMLElement->text) == 0) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_TEXT_IS_NIL));
        return "";
    }

	std::string rev(aXMLElement->text);
	return rev;
}

std::string TBXML::valueOfAttributeNamed(std::string aName, TBXMLElement* aXMLElement) {
	const char* name = aName.c_str();
	char* value;
	TBXMLAttribute * attribute = aXMLElement->firstAttribute;
	while (attribute) {
		if (strlen(attribute->name) == strlen(name) && memcmp(attribute->name,name,strlen(name)) == 0) {
			value = attribute->value;
			break;
		}
		attribute = attribute->next;
	}

	std::string rev(value);
	return rev;
}

std::string TBXML::valueOfAttributeNamed(std::string aName, TBXMLElement* aXMLElement, std::string error) {
    // check for nil element
    if (NULL == aXMLElement) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
        return "";
    }
    
    // check for nil name parameter
    if (aName.length() == 0) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ATTRIBUTE_NAME_IS_NIL));
        return "";
    }
    
    const char* name = aName.c_str();
    char* value;
    
	TBXMLAttribute * attribute = aXMLElement->firstAttribute;
	while (attribute) {
		if (strlen(attribute->name) == strlen(name) && memcmp(attribute->name,name,strlen(name)) == 0) {
            if (attribute->value)
    			value = attribute->value;
            else
                value = (char*)"";
            
			break;
		}
		attribute = attribute->next;
	}
    
    // check for attribute not found
    if (!value) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ATTRIBUTE_NOT_FOUND));
        return "";
    }

	std::string rev(value);
	return rev;
}

TBXMLElement* TBXML::childElementNamed(std::string aName, TBXMLElement* aParentXMLElement) {
	TBXMLElement * xmlElement = aParentXMLElement->firstChild;
	const char * name = aName.c_str();
	while (xmlElement) {
		if (strlen(xmlElement->name) == strlen(name) && memcmp(xmlElement->name,name,strlen(name)) == 0) {
			return xmlElement;
		}
		xmlElement = xmlElement->nextSibling;
	}
	return NULL;
}

TBXMLElement* TBXML::childElementNamed(std::string aName, TBXMLElement* aParentXMLElement, std::string error) {
    // check for nil element
    if (NULL == aParentXMLElement) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
        return NULL;
    }
    
    // check for nil name parameter
    if (aName.length() == 0) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_PARAM_NAME_IS_NIL));
        return NULL;
    }
    
	TBXMLElement * xmlElement = aParentXMLElement->firstChild;
	const char * name = aName.c_str();
	while (xmlElement) {
		if (strlen(xmlElement->name) == strlen(name) && memcmp(xmlElement->name,name,strlen(name)) == 0) {
			return xmlElement;
		}
		xmlElement = xmlElement->nextSibling;
	}

	error.clear();
	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
    return NULL;
}

TBXMLElement* TBXML::nextSiblingNamed(std::string aName, TBXMLElement* aXMLElement) {
	TBXMLElement * xmlElement = aXMLElement->nextSibling;
	const char * name = aName.c_str();
	while (xmlElement) {
		if (strlen(xmlElement->name) == strlen(name) && memcmp(xmlElement->name,name,strlen(name)) == 0) {
			return xmlElement;
		}
		xmlElement = xmlElement->nextSibling;
	}
	return NULL;
}

TBXMLElement* TBXML::nextSiblingNamed(std::string aName, TBXMLElement* aXMLElement, std::string error) {
    // check for nil element
    if (NULL == aXMLElement) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_IS_NIL));
    	return NULL;
    }
    
    // check for nil name parameter
    if (aName.length() == 0) {
    	error.clear();
    	error.append(TBXML::errorWithCode(D_TBXML_PARAM_NAME_IS_NIL));
    	return NULL;
    }
    
	TBXMLElement * xmlElement = aXMLElement->nextSibling;
	const char * name = aName.c_str();
	while (xmlElement) {
		if (strlen(xmlElement->name) == strlen(name) && memcmp(xmlElement->name,name,strlen(name)) == 0) {
			return xmlElement;
		}
		xmlElement = xmlElement->nextSibling;
	}

	error.clear();
	error.append(TBXML::errorWithCode(D_TBXML_ELEMENT_NOT_FOUND));
	return NULL;
}

std::string TBXML::errorWithCode(int code) {
    std::string codeText = "";
    
    switch (code) {
        case D_TBXML_DATA_NIL:                  codeText = "Data is nil";                          break;
        case D_TBXML_DECODE_FAILURE:            codeText = "Decode failure";                       break;
        case D_TBXML_MEMORY_ALLOC_FAILURE:      codeText = "Unable to allocate memory";            break;
        case D_TBXML_FILE_NOT_FOUND_IN_BUNDLE:  codeText = "File not found in bundle";             break;
            
        case D_TBXML_ELEMENT_IS_NIL:            codeText = "Element is nil";                       break;
        case D_TBXML_ELEMENT_NAME_IS_NIL:       codeText = "Element name is nil";                  break;
        case D_TBXML_ATTRIBUTE_IS_NIL:          codeText = "Attribute is nil";                     break;
        case D_TBXML_ATTRIBUTE_NAME_IS_NIL:     codeText = "Attribute name is nil";                break;
        case D_TBXML_ELEMENT_TEXT_IS_NIL:       codeText = "Element text is nil";                  break;
        case D_TBXML_PARAM_NAME_IS_NIL:         codeText = "Parameter name is nil";                break;
        case D_TBXML_ATTRIBUTE_NOT_FOUND:       codeText = "Attribute not found";                  break;
        case D_TBXML_ELEMENT_NOT_FOUND:         codeText = "Element not found";                    break;
            
        default: codeText = "No Error Description!"; break;
    }
    
    return std::string(codeText);
}

void TBXML::decodeBytes() {
	
	// -----------------------------------------------------------------------------
	// Process xml
	// -----------------------------------------------------------------------------
	
	// set elementStart pointer to the start of our xml
	char * elementStart=bytes;
	
	// set parent element to nil
	TBXMLElement * parentXMLElement = NULL;
	
	// find next element start
	while ((elementStart = strstr(elementStart,"<"))) {
		
		// detect comment section
		if (strncmp(elementStart,"<!--",4) == 0) {
			elementStart = strstr(elementStart,"-->") + 3;
			continue;
		}

		// detect cdata section within element text
		int isCDATA = strncmp(elementStart,"<![CDATA[",9);
		
		// if cdata section found, skip data within cdata section and remove cdata tags
		if (isCDATA==0) {
			
			// find end of cdata section
			char * CDATAEnd = strstr(elementStart,"]]>");
			
			// find start of next element skipping any cdata sections within text
			char * elementEnd = CDATAEnd;
			
			// find next open tag
			elementEnd = strstr(elementEnd,"<");
			// if open tag is a cdata section
			while (strncmp(elementEnd,"<![CDATA[",9) == 0) {
				// find end of cdata section
				elementEnd = strstr(elementEnd,"]]>");
				// find next open tag
				elementEnd = strstr(elementEnd,"<");
			}
			
			// calculate length of cdata content
			long CDATALength = CDATAEnd-elementStart;
			
			// calculate total length of text
			long textLength = elementEnd-elementStart;
			
			// remove begining cdata section tag
			memcpy(elementStart, elementStart+9, CDATAEnd-elementStart-9);

			// remove ending cdata section tag
			memcpy(CDATAEnd-9, CDATAEnd+3, textLength-CDATALength-3);
			
			// blank out end of text
			memset(elementStart+textLength-12,' ',12);
			
			// set new search start position 
			elementStart = CDATAEnd-9;
			continue;
		}
		
		
		// find element end, skipping any cdata sections within attributes
		char * elementEnd = elementStart+1;		
		while ((elementEnd = strpbrk(elementEnd, "<>"))) {
			if (strncmp(elementEnd,"<![CDATA[",9) == 0) {
				elementEnd = strstr(elementEnd,"]]>")+3;
			} else {
				break;
			}
		}
		
        if (!elementEnd) break;
		
		// null terminate element end
		if (elementEnd) *elementEnd = 0;
		
		// null terminate element start so previous element text doesnt overrun
		*elementStart = 0;
		
		// get element name start
		char * elementNameStart = elementStart+1;
		
		// ignore tags that start with ? or ! unless cdata "<![CDATA"
		if (*elementNameStart == '?' || (*elementNameStart == '!' && isCDATA != 0)) {
			elementStart = elementEnd+1;
			continue;
		}
		
		// ignore attributes/text if this is a closing element
		if (*elementNameStart == '/') {
			elementStart = elementEnd+1;
			if (parentXMLElement) {

                // Compare opening and closing strings
                if( *(elementNameStart+1) != 0 && strcmp(parentXMLElement->name,(elementNameStart+1))  != 0 ){
                	char e[100];
                	sprintf(e, "XML Element doesn't have matching tags, : %s != %s", parentXMLElement->name, elementNameStart+1);
                    assert(e);
                }
                   
				if (parentXMLElement->text) {
					// trim whitespace from start of text
					while (isspace(*parentXMLElement->text)) 
						parentXMLElement->text++;
					
					// trim whitespace from end of text
					char * end = parentXMLElement->text + strlen(parentXMLElement->text)-1;
					while (end > parentXMLElement->text && isspace(*end)) 
						*end--=0;
				}
				
				parentXMLElement = parentXMLElement->parentElement;
				
				// if parent element has children clear text
				if (parentXMLElement && parentXMLElement->firstChild)
					parentXMLElement->text = 0;
				
			}
			continue;
		}
		
		
		// is this element opening and closing
		bool selfClosingElement = false;
		if (*(elementEnd-1) == '/') {
			selfClosingElement = true;
		}
		
		
		// create new xmlElement struct
		TBXMLElement * xmlElement = this->nextAvailableElement();
		
		// set element name
		xmlElement->name = elementNameStart;
		
		// if there is a parent element
		if (parentXMLElement) {
			
			// if this is first child of parent element
			if (parentXMLElement->currentChild) {
				// set next child element in list
				parentXMLElement->currentChild->nextSibling = xmlElement;
				xmlElement->previousSibling = parentXMLElement->currentChild;
				
				parentXMLElement->currentChild = xmlElement;
				
				
			} else {
				// set first child element
				parentXMLElement->currentChild = xmlElement;
				parentXMLElement->firstChild = xmlElement;
			}
			
			xmlElement->parentElement = parentXMLElement;
		}
		
		
		// in the following xml the ">" is replaced with \0 by elementEnd. 
		// element may contain no atributes and would return nil while looking for element name end
		// <tile> 
		// find end of element name
		char * elementNameEnd = strpbrk(xmlElement->name," /\n");
		
		
		// if end was found check for attributes
		if (elementNameEnd) {
			
			// null terminate end of elemenet name
			*elementNameEnd = 0;
			
			char * chr = elementNameEnd;
			char * name = NULL;
			char * value = NULL;
			char * CDATAStart = NULL;
			char * CDATAEnd = NULL;
			TBXMLAttribute * lastXMLAttribute = NULL;
			TBXMLAttribute * xmlAttribute = NULL;
			bool singleQuote = false;
			
			int mode = TBXML_ATTRIBUTE_NAME_START;
			
			// loop through all characters within element
			while (chr++ < elementEnd) {
				
				switch (mode) {
					// look for start of attribute name
					case TBXML_ATTRIBUTE_NAME_START:
						if (isspace(*chr)) continue;
						name = chr;
						mode = TBXML_ATTRIBUTE_NAME_END;
						break;
					// look for end of attribute name
					case TBXML_ATTRIBUTE_NAME_END:
						if (isspace(*chr) || *chr == '=') {
							*chr = 0;
							mode = TBXML_ATTRIBUTE_VALUE_START;
						}
						break;
					// look for start of attribute value
					case TBXML_ATTRIBUTE_VALUE_START:
						if (isspace(*chr)) continue;
						if (*chr == '"' || *chr == '\'') {
							value = chr+1;
							mode = TBXML_ATTRIBUTE_VALUE_END;
							if (*chr == '\'') 
								singleQuote = true;
							else
								singleQuote = false;
						}
						break;
					// look for end of attribute value
					case TBXML_ATTRIBUTE_VALUE_END:
						if (*chr == '<' && strncmp(chr, "<![CDATA[", 9) == 0) {
							mode = TBXML_ATTRIBUTE_CDATA_END;
						}else if ((*chr == '"' && !singleQuote) || (*chr == '\'' && singleQuote)) {
							*chr = 0;
							
							// remove cdata section tags
							while ((CDATAStart = strstr(value, "<![CDATA["))) {
								
								// remove begin cdata tag
								memcpy(CDATAStart, CDATAStart+9, strlen(CDATAStart)-8);
								
								// search for end cdata
								CDATAEnd = strstr(CDATAStart,"]]>");
								
								// remove end cdata tag
								memcpy(CDATAEnd, CDATAEnd+3, strlen(CDATAEnd)-2);
							}
							
							
							// create new attribute
							xmlAttribute = this->nextAvailableAttribute();
							
							// if this is the first attribute found, set pointer to this attribute on element
							if (!xmlElement->firstAttribute) xmlElement->firstAttribute = xmlAttribute;
							// if previous attribute found, link this attribute to previous one
							if (lastXMLAttribute) lastXMLAttribute->next = xmlAttribute;
							// set last attribute to this attribute
							lastXMLAttribute = xmlAttribute;

							// set attribute name & value
							xmlAttribute->name = name;
							xmlAttribute->value = value;
							
							// clear name and value pointers
							name = NULL;
							value = NULL;
							
							// start looking for next attribute
							mode = TBXML_ATTRIBUTE_NAME_START;
						}
						break;
						// look for end of cdata
					case TBXML_ATTRIBUTE_CDATA_END:
						if (*chr == ']') {
							if (strncmp(chr, "]]>", 3) == 0) {
								mode = TBXML_ATTRIBUTE_VALUE_END;
							}
						}
						break;						
					default:
						break;
				}
			}
		}
		
		// if tag is not self closing, set parent to current element
		if (!selfClosingElement) {
			// set text on element to element end+1
			if (*(elementEnd+1) != '>')
				xmlElement->text = elementEnd+1;
			
			parentXMLElement = xmlElement;
		}
		
		// start looking for next element after end of current element
		elementStart = elementEnd+1;
	}
}

TBXMLElement* TBXML::nextAvailableElement() {
	currentElement++;

	if (!currentElementBuffer) {
		currentElementBuffer = (TBXMLElementBuffer*)calloc(1, sizeof(TBXMLElementBuffer));
		currentElementBuffer->elements = (TBXMLElement*)calloc(1,sizeof(TBXMLElement)*MAX_ELEMENTS);
		currentElement = 0;
		rootXMLElement = &currentElementBuffer->elements[currentElement];
	} else if (currentElement >= MAX_ELEMENTS) {
		currentElementBuffer->next = (TBXMLElementBuffer*)calloc(1, sizeof(TBXMLElementBuffer));
		currentElementBuffer->next->previous = currentElementBuffer;
		currentElementBuffer = currentElementBuffer->next;
		currentElementBuffer->elements = (TBXMLElement*)calloc(1,sizeof(TBXMLElement)*MAX_ELEMENTS);
		currentElement = 0;
	}

	return &currentElementBuffer->elements[currentElement];
}

TBXMLAttribute* TBXML::nextAvailableAttribute() {
	currentAttribute++;

	if (!currentAttributeBuffer) {
		currentAttributeBuffer = (TBXMLAttributeBuffer*)calloc(1, sizeof(TBXMLAttributeBuffer));
		currentAttributeBuffer->attributes = (TBXMLAttribute*)calloc(MAX_ATTRIBUTES,sizeof(TBXMLAttribute));
		currentAttribute = 0;
	} else if (currentAttribute >= MAX_ATTRIBUTES) {
		currentAttributeBuffer->next = (TBXMLAttributeBuffer*)calloc(1, sizeof(TBXMLAttributeBuffer));
		currentAttributeBuffer->next->previous = currentAttributeBuffer;
		currentAttributeBuffer = currentAttributeBuffer->next;
		currentAttributeBuffer->attributes = (TBXMLAttribute*)calloc(MAX_ATTRIBUTES,sizeof(TBXMLAttribute));
		currentAttribute = 0;
	}

	return &currentAttributeBuffer->attributes[currentAttribute];
}
