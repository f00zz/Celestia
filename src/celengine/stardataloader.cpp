
#include <celutil/util.h>
#include <fmt/printf.h>
#include "parseobject.h"
#include "stardataloader.h"
#include "astrocat.h"
#include "star.h"
#include "astrodb.h"

#undef NDEBUG

using namespace std;

/*! Load an STC file with star definitions. Each definition has the form:
 *
 *  [disposition] [object type] [catalog number] [name]
 *  {
 *      [properties]
 *  }
 *
 *  Disposition is either Add, Replace, or Modify; Add is the default.
 *  Object type is either Star or Barycenter, with Star the default
 *  It is an error to omit both the catalog number and the name.
 *
 *  The dispositions are slightly more complicated than suggested by
 *  their names. Every star must have an unique catalog number. But
 *  instead of generating an error, Adding a star with a catalog
 *  number that already exists will actually replace that star. Here
 *  are how all of the possibilities are handled:
 *
 *  <name> or <number> already exists:
 *  Add <name>        : new star
 *  Add <number>      : replace star
 *  Replace <name>    : replace star
 *  Replace <number>  : replace star
 *  Modify <name>     : modify star
 *  Modify <number>   : modify star
 *
 *  <name> or <number> doesn't exist:
 *  Add <name>        : new star
 *  Add <number>      : new star
 *  Replace <name>    : new star
 *  Replace <number>  : new star
 *  Modify <name>     : error
 *  Modify <number>   : error
 */
bool StcDataLoader::load(istream &in)
{
    Tokenizer tokenizer(&in);
    Parser parser(&tokenizer);

    bindtextdomain(resourcePath.c_str(), resourcePath.c_str()); // domain name is the same as resource path

    while (tokenizer.nextToken() != Tokenizer::TokenEnd)
    {
        bool isStar = true;

        // Parse the disposition--either Add, Replace, or Modify. The disposition
        // may be omitted. The default value is Add.
        DataDisposition disposition = DataDisposition::Add;
        if (tokenizer.getTokenType() == Tokenizer::TokenName)
        {
            if (tokenizer.getNameValue() == "Modify")
            {
//                 clog << "New entry with disposition: Modify.\n";
                disposition = DataDisposition::Modify;
                tokenizer.nextToken();
            }
            else if (tokenizer.getNameValue() == "Replace")
            {
//                 clog << "New entry with disposition: Replace.\n";
                disposition = DataDisposition::Replace;
                tokenizer.nextToken();
            }
            else if (tokenizer.getNameValue() == "Add")
            {
//                 clog << "New entry with disposition: Add.\n";
                disposition = DataDisposition::Add;
                tokenizer.nextToken();
            }// else clog << "New entry with unrecognized disposition type: " << tokenizer.getNameValue() << endl;
        }// else clog << "New entry with default disposition: Add.\n";

        // Parse the object type--either Star or Barycenter. The object type
        // may be omitted. The default is Star.
        if (tokenizer.getTokenType() == Tokenizer::TokenName)
        {
            if (tokenizer.getNameValue() == "Star")
            {
                isStar = true;
            }
            else if (tokenizer.getNameValue() == "Barycenter")
            {
                isStar = false;
            }
            else
            {
                stcError(tokenizer, "unrecognized object type");
                return false;
            }
            tokenizer.nextToken();
        }

//         clog << " Is star: " << isStar << endl;
        // Parse the catalog number; it may be omitted if a name is supplied.
        AstroCatalog::IndexNumber catalogNumber = AstroCatalog::InvalidIndex;
        if (tokenizer.getTokenType() == Tokenizer::TokenNumber)
        {
            catalogNumber = (AstroCatalog::IndexNumber) tokenizer.getNumberValue();
            tokenizer.nextToken();
        }

        string objName;
        string firstName;
        if (tokenizer.getTokenType() == Tokenizer::TokenString)
        {
            // A star name (or names) is present
            objName    = tokenizer.getStringValue();
            tokenizer.nextToken();
            if (!objName.empty())
            {
                string::size_type next = objName.find(':', 0);
                firstName = objName.substr(0, next);
            }
        }

        bool isNewStar = false;
        bool ok = true;
        Star* star = nullptr;

        switch (disposition)
        {
        case DataDisposition::Add:
            // Automatically generate a catalog number for the star if one isn't
            // supplied.
            star = new Star();
            star->setMainIndexNumber(catalogNumber);
            isNewStar = true;
//             clog << " Add: About to add star with nr: " << catalogNumber << endl;
            if (m_db->addStar(star))
            {
                catalogNumber = star->getMainIndexNumber();
//                 clog << " Add: Added star with nr " << catalogNumber << endl;
            }
            else
            {
//                 clog << " Unable to add star with index " << star->getMainIndexNumber() << endl;
                delete star;
                ok = false;
            }
            break;

        case DataDisposition::Replace:
            if (catalogNumber == AstroCatalog::InvalidIndex)
            {
                if (!firstName.empty())
                {
                    catalogNumber = m_db->findCatalogNumberByName(firstName);
                }
            }

            if (catalogNumber == AstroCatalog::InvalidIndex)
            {
                star = new Star();
//                 clog << " Replace: About to add star with nr: " << catalogNumber << endl;
                if (m_db->addStar(star))
                {
                    catalogNumber = star->getMainIndexNumber();
//                     clog << " Replace: Added star with nr " << catalogNumber << endl;
                }
                else
                {
//                     clog << " Unable to add star with index " << star->getMainIndexNumber() << endl;
                    delete star;
                    ok = false;
                }
                isNewStar = true;
            }
            else
            {
                star = m_db->getStar(catalogNumber);
                if (star == nullptr)
                {
                    star = new Star();
                    star->setMainIndexNumber(catalogNumber);
//                     clog << " Replace 2: About to add star with nr: " << catalogNumber << endl;
                    if (!m_db->addStar(star))
                    {
//                         clog << " Unable to add star with index " << star->getMainIndexNumber() << endl;
                        delete star;
                        ok = false;
                    }// else clog << " Replace 2: Added star with nr " << catalogNumber << endl;
                }
            }
            break;

        case DataDisposition::Modify:
            // If no catalog number was specified, try looking up the star by name
            if (catalogNumber == AstroCatalog::InvalidIndex && !firstName.empty())
            {
                catalogNumber = m_db->findCatalogNumberByName(firstName);
            }

            if (catalogNumber == AstroCatalog::InvalidIndex)
            {
//                 clog << " No star index to modify.\n";
                ok = false;
                break;
            }
            star = m_db->getStar(catalogNumber);
            if (star == nullptr)
            {
//                 clog << " Unable to find star with index " << catalogNumber << ".\n";
                ok = false;
            }
            break;
        }

        tokenizer.pushBack();

        Value* starDataValue = parser.readValue();
        if (starDataValue == nullptr)
        {
            clog << " Error reading star.\n";
            return false;
        }

        if (starDataValue->getType() != Value::HashType)
        {
//            clog << " Bad star definition.\n";
            delete starDataValue;
            return false;
        }
        Hash* starData = starDataValue->getHash();

/*        if (isNewStar)
            star = new Star();*/

        if (ok)
        {
//            clog << " About to create star with nr " << star->getMainIndexNumber() << endl;
            ok = Star::createStar(star, disposition, starData, resourcePath, !isStar, m_db);
            if (star->getDetails() == nullptr)
//                 clog << " Created star has null details!\n";
            star->loadCategories(starData, disposition, resourcePath);
        } //else clog << " Errors while preparing star, skipping creation.\n";
        delete starDataValue;

        if (ok)
        {

            if (!objName.empty())
            {
                // List of namesDB will replace any that already exist for
                // this star.
                m_db->eraseNames(catalogNumber);

                // Iterate through the string for names delimited
                // by ':', and insert them into the star database.
                // Note that db->add() will skip empty namesDB.
                string::size_type startPos = 0;
                while (startPos != string::npos)
                {
                    string::size_type next    = objName.find(':', startPos);
                    string::size_type length = string::npos;
                    if (next != string::npos)
                    {
                        length = next - startPos;
                        ++next;
                    }
                    string starName = objName.substr(startPos, length);
                    m_db->addName(catalogNumber, starName);
                    clog << " Adding name \"" << starName << "\" for entry nr " << catalogNumber << endl;
                    if (starName != _(starName.c_str()))
                        m_db->addName(catalogNumber, _(starName.c_str()));
                    startPos = next;
                }
            }
        }
        else
        {
            if (isNewStar)
                delete star;
//             clog << "Bad star definition -- will continue parsing file.\n";
        }
//         clog << "End parsing entry.\n";
    }

    return true;
}
