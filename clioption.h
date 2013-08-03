/*!
 * \file
 * C++ interface for the \c CliOption class.
 *
 * \author Stephen Bryant <steve@bawue.de>
 * \date $Date: 2010-09-24 00:22:44 +0200 (Fri, 24 Sep 2010) $
 */

#ifndef CliOption_H
#define CliOption_H

#include <QString>
#include <QStringList>
#include <QSet>


/*!
 * \class CliOption clioption.h
 * \brief An option on the command line interface.
 * \author Stephen Bryant <steve@bawue.de>
 *
 * This class is used to describe an option on the command line.  It allows
 * for different ways of defining the same option - long and short, with
 * multiple aliases possible.  It is also used to describe how the option
 * is used - it may be an on/off switch or take an argument etc.
 *
 * \sa CliParser
 */

class CliOption
{
public:

    //! Default constructor.
    CliOption();

    //! Constructor with initialisation.
    CliOption( const QStringList & names, const bool bHasArg = false );


    //! Set with the named of long options.
    QSet< QString > m_longnameSet;

    //! Set with the named of short options.
    QSet< QChar > m_shortnameSet;

    //! Whether the option takes an argument.
    bool m_bHasArg;


    /*!
     * Set the names to use for this option.
     *
     * Every option must have at least one name; it can be either short or
     * long.  The names specified must not start with the dash character.
     *
     * Any name in the list that is one character in length is a short
     * name.  Those which are 2 characters or longer are long names.  Zero
     * length and duplicate names are ignored.  An option name may not start
     * with a dash character (these are also ignored).
     *
     * The names are added to either \c m_longnameSet or \c m_shortnameSet.
     * Existing entries in those sets are not removed.
     *
     * \param names a list of names to use for this option.
     */
    void setNames( const QStringList & names );

};

#endif // CliOption_H
