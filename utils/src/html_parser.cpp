#include "html_parser.hpp"
#include "logger.hpp"


void granada::utils::HtmlParser::searchDivByClass(GumboNode* node, const std::string& class_name, int& index, int target_index) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    GumboElement* element = &node->v.element;

    // Check if the element is a <div> and has the specified class
    if (element->tag == GUMBO_TAG_DIV) {
        bool has_class = false;
        for (int i = 0; i < element->attributes.length; ++i) {
            GumboAttribute* attr = (GumboAttribute*)element->attributes.data[i];
            const std::string name = attr->name;
            const std::string value = attr->value;
            if (name == "class" && value == class_name) {
                has_class = true;
                LOG_DEBUG("FOUND div.");
                for (int i = 0; i < element->children.length; ++i) {
                    GumboElement* child = (GumboElement*)element->children.data[i];
                    LOG_DEBUG(child->tag);
                    if (child->tag == GUMBO_TAG_A) {
                        GumboNode* child_child = (GumboNode*)child->children.data[i];
                        if (child_child->type == GUMBO_NODE_TEXT) {
                            LOG_DEBUG_FMT("Read tag content: {}", child_child->v.text.text);
                        }
                    }
                }
                
                break;
            }   
        }

        if (has_class) {
            index++;
            if (index == target_index) {
                for (int i = 0; i < element->children.length; ++i) {
                    GumboNode* child = (GumboNode*)element->children.data[i];
                    if (child->type == GUMBO_NODE_TEXT) {
                        LOG_DEBUG_FMT("Read tag content: {}", child->v.text.text);
                    }
                }
            }
        }
    }

    // Recursively search the children
    for (int i = 0; i < element->children.length; ++i) {
        searchDivByClass((GumboNode*)element->children.data[i], class_name, i, target_index);
    }
}

void granada::utils::HtmlParser::getTitle(HTML& text, std::string & title)
{

    GumboOutput *output = gumbo_parse(text.c_str());
    searchTitleElement(output->root, title);

    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void granada::utils::HtmlParser::getDivContent(HTML &text, const std::string &class_name, int target_index)
{
    GumboOutput* output = gumbo_parse(text.c_str());
    
    int index = 0;
    searchDivByClass(output->root, class_name, index, target_index);

    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void granada::utils::HtmlParser::searchTitleElement(GumboNode *node, std::string & title)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboElement *element = &node->v.element;

    if (element->tag == GUMBO_TAG_TITLE)
    {
        for (size_t i = 0; i < element->children.length; ++i)
        {
            GumboNode *child = (GumboNode *)element->children.data[i];
            if (child->type == GUMBO_NODE_TEXT)
            {
                LOG_DEBUG_FMT("Title: {}", child->v.text.text);
                title = child->v.text.text;
            }
        }
    }

    for (size_t i = 0; i < element->children.length; ++i)
    {
        searchTitleElement((GumboNode *)element->children.data[i], title);
    }
}
