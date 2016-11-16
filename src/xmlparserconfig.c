/**
 * section: 	XPath
 * synopsis: 	Evaluate XPath expression and prints result node set.
 * purpose: 	Shows how to evaluate XPath expression and register
 *          	known namespaces in XPath context.
 * usage:	xpath1 <xml-file> <xpath-expr> [<known-ns-list>]
 * test: ./xpath1 test3.xml '//child2' > xpath1.tmp ; diff xpath1.tmp xpath1.res ; rm xpath1.tmp
 * author: 	Aleksey Sanin
 * copy: 	see Copyright for the status of this software.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlreader.h>


#include "dictionary.h"
#include "xmlparserconfig.h"


xmlChar* xgetPropertyCommunity(xmlDocPtr doc, xmlChar *exp, xmlChar* property){
	xmlXPathObjectPtr xpathObj;
	xmlChar *xvalue=NULL;

	xpathObj=execute_xpath_expression(doc, BAD_CAST exp);
	if (xpathObj->nodesetval){
		xvalue = xmlGetProp(xpathObj->nodesetval->nodeTab[0], property);
	}
	xmlXPathFreeObject(xpathObj);
	return xvalue;
}


void* xgetOneParameter(xmlDocPtr doc, char *xpath){
	xmlNodeSetPtr nodes;
	xmlXPathObjectPtr xpathObj;
	xmlChar *content=NULL;

	int size;
	int i;

	xpathObj = execute_xpath_expression(doc, BAD_CAST xpath);

	nodes = xpathObj->nodesetval;
	size = (nodes) ? nodes->nodeNr : 0;
	for(i = 0; i < size; ++i) {
		assert(nodes->nodeTab[i]);

		if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {

			xmlNodePtr cur = nodes->nodeTab[i];
			if(cur->ns) {
				fprintf(stderr, "ERROR= element node \"%s:%s\"\n",
						cur->ns->href, cur->name);
			} else {
				content = xmlNodeGetContent(cur);
			}
		}

	}
	xmlXPathFreeObject(xpathObj);

	return content;

}


/**
 * execute_xpath_expression:
 * @filename:		the input XML filename.
 * @xpathExpr:		the xpath expression for evaluation.
 * @nsList:		the optional list of known namespaces in
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Parses input XML file, evaluates XPath expression and prints results.
 *
 * Returns 0 on success and a negative value otherwise.
 */
xmlXPathObjectPtr execute_xpath_expression(xmlDocPtr doc, const xmlChar* xpathExpr) {
	const xmlChar* nsList=NULL;
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;

	assert(xpathExpr);

	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if(xpathCtx == NULL) {
		fprintf(stderr,"Error: unable to create new XPath context\n");
		return(NULL);
	}

	/* Register namespaces from list (if any) */
	if((nsList != NULL) && (register_namespaces(xpathCtx, nsList) < 0)) {
		fprintf(stderr,"Error: failed to register namespaces list \"%s\"\n", nsList);
		xmlXPathFreeContext(xpathCtx);
		return(NULL);
	}

	/* Evaluate xpath expression */
	xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if(xpathObj == NULL) {
		fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
		xmlXPathFreeContext(xpathCtx);
		return(NULL);
	}

	/* Cleanup */
	xmlXPathFreeContext(xpathCtx);

	return xpathObj;
}

/**
 * register_namespaces:
 * @xpathCtx:		the pointer to an XPath context.
 * @nsList:		the list of known namespaces in
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 0 on success and a negative value otherwise.
 */
int
register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
	xmlChar* nsListDup;
	xmlChar* prefix;
	xmlChar* href;
	xmlChar* next;

	assert(xpathCtx);
	assert(nsList);

	nsListDup = xmlStrdup(nsList);
	if(nsListDup == NULL) {
		fprintf(stderr, "Error: unable to strdup namespaces list\n");
		return(-1);
	}

	next = nsListDup;
	while(next != NULL) {
		/* skip spaces */
		while((*next) == ' ') next++;
		if((*next) == '\0') break;

		/* find prefix */
		prefix = next;
		next = (xmlChar*)xmlStrchr(next, '=');
		if(next == NULL) {
			fprintf(stderr,"Error: invalid namespaces list format\n");
			xmlFree(nsListDup);
			return(-1);
		}
		*(next++) = '\0';

		/* find href */
		href = next;
		next = (xmlChar*)xmlStrchr(next, ' ');
		if(next != NULL) {
			*(next++) = '\0';
		}

		/* do register namespace */
		if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0) {
			fprintf(stderr,"Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
			xmlFree(nsListDup);
			return(-1);
		}
	}

	xmlFree(nsListDup);
	return(0);
}

/**
 * print_xpath_nodes:
 * @nodes:		the nodes set.
 * @output:		the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void
print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
	xmlNodePtr cur;
	int size;
	int i;

	assert(output);
	size = (nodes) ? nodes->nodeNr : 0;

	fprintf(output, "Result (%d nodes):\n", size);
	for(i = 0; i < size; ++i) {
		assert(nodes->nodeTab[i]);

		if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
			xmlNsPtr ns;

			ns = (xmlNsPtr)nodes->nodeTab[i];
			cur = (xmlNodePtr)ns->next;
			if(cur->ns) {
				fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n",
						ns->prefix, ns->href, cur->ns->href, cur->name);
			} else {
				fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n",
						ns->prefix, ns->href, cur->name);
			}
		} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
			cur = nodes->nodeTab[i];
			if(cur->ns) {
				fprintf(output, "= element node \"%s:%s\"\n",
						cur->ns->href, cur->name);
			} else {

				fprintf(output, "= name: %s; content: %d %s\n",
						cur->name, xmlNodeIsText(cur), xmlNodeGetContent(cur));
			}
		} else {
			cur = nodes->nodeTab[i];
			fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
		}
	}
}

xmlDocPtr extract_subdocument(xmlTextReaderPtr reader, const xmlChar *pattern) {
	xmlDocPtr doc;
	int ret;

// * build an xmlReader for that file
	if (reader != NULL) {

 //* add the pattern to preserve

		if (xmlTextReaderPreservePattern(reader, pattern, NULL) < 0) {
			fprintf(stderr, ": failed add preserve pattern %s\n", (const char *) pattern);
		}

 //* Parse and traverse the tree, collecting the nodes in the process

		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			ret = xmlTextReaderRead(reader);
		}
		if (ret != 0) {
			fprintf(stderr, ": failed to parse\n");
			xmlFreeTextReader(reader);
			return (NULL);
		}

// * get the resulting nodes

		doc = xmlTextReaderCurrentDoc(reader);

// * Free up the reader

//		xmlFreeTextReader(reader);
	} else {
		fprintf(stderr, "Unable to open \n");
		return (NULL);
	}
	return (doc);
}

