/*!
 * \file
 * C++ implementation for the \c CliParser class.
 *
 * \author Stephen Bryant <steve@bawue.de>
 * \date $Date: 2010-12-14 16:46:09 +0100 (Tue, 14 Dec 2010) $
 */

#include "cliparser.h"

#include <QCoreApplication>


CliParser::CliParser()
  : m_bStopParsingAtDoubleDash( true ),
    m_bRemoveUnknownLongNames( true )
{
}


bool CliParser::addOption( const CliOption & option )
{
    bool bOptionAdded = false;

    if ( ( m_optionList.size() < m_maxOptionCount ) &&
         ! ( option.m_longnameSet.isEmpty() &&
             option.m_shortnameSet.isEmpty() ) )
    {
        // Whether a duplicate name has been found.
        bool bDuplicateName = false;

        QSet< QString >::const_iterator longIter;

        // Check for duplicate names.
        for ( longIter =  option.m_longnameSet.constBegin();
              longIter != option.m_longnameSet.constEnd();
              longIter ++ )
        {
            if ( m_nameHash.contains( *longIter ) )
            {
                bDuplicateName = true;
                break;
            }
        }

        if ( ! bDuplicateName )
        {
            QSet< QChar >::const_iterator shortIter;

            // Check for duplicate names.
            for ( shortIter =  option.m_shortnameSet.constBegin();
                  shortIter != option.m_shortnameSet.constEnd();
                  shortIter ++ )
            {
                if ( m_nameHash.contains( *shortIter ) )
                {
                    bDuplicateName = true;
                    break;
                }
            }


            if ( ! bDuplicateName )
            {
                // Add it
                try
                {
                    m_optionList.append( option );
                    const quint16 offset = (quint16) ( m_optionList.size() - 1 );

                    for ( longIter =  option.m_longnameSet.constBegin();
                          longIter != option.m_longnameSet.constEnd();
                          longIter ++ )
                    {
                        m_nameHash.insert( *longIter, offset );
                    }

                    for ( shortIter =  option.m_shortnameSet.constBegin();
                          shortIter != option.m_shortnameSet.constEnd();
                          shortIter ++ )
                    {
                        m_nameHash.insert( *shortIter, offset );
                    }

                    bOptionAdded = true;
                }
                catch ( std::bad_alloc & )
                {
                }
            } // if no duplicate name found
        } // if no duplicate name found
    } // if not too many options and option has a name

    return bOptionAdded;
}


bool CliParser::parse()
{
    bool bParseOk = false;

    QCoreApplication *pApp = QCoreApplication::instance();

    if ( NULL != pApp )
    {
        QStringList args = pApp->arguments();

        if ( ! args.isEmpty() )
        {
            args.removeFirst();
            bParseOk = parse( args );
        }
    }

    return bParseOk;

} // CliParser::parse()


bool CliParser::parse( const QStringList & arguments )
{
    bool bParseOk = false;

    const int argumentsSize = arguments.size();
    int       argumentsOffset;
    int       argSize;
    int       assignOffset;
    int       i;
    QString   name;
    bool      bParsingStopped = false;

    NameHash_t::const_iterator nameIter;
    NameHash_t::mapped_type    optionOffset;

    static const QStringList emptyList;
    static const QString     emptyString( QLatin1String( "" ) );
    static const QLatin1Char dashChar( '-' );
    static const QLatin1Char assignChar( '=' );

    m_leftoverArgs.clear();
    m_optionNames.clear();
    m_unknownOptionNames.clear();
    m_optionArgsList.clear();

    try
    {
        // Prepare an empty list of arguments for each option - as nothing
        // has been found yet.
        for ( i = m_optionList.size(); i > 0; i -- )
        {
            m_optionArgsList.append( emptyList );
        }


        for ( argumentsOffset = 0;
              argumentsOffset < argumentsSize;
              argumentsOffset ++ )
        {
            const QString & arg = arguments.at( argumentsOffset );

            argSize = arg.size();

            if ( bParsingStopped ||
                 //arg.isEmpty() ||
                 ( 1 >= argSize ) ||
                 ( arg.at( 0 ) != dashChar ) )
            {
                // Not an option
                m_leftoverArgs.append( arg );
            }
            else if ( arg.at( 1 ) != dashChar )
            {
                // Short option
                for ( i = 1; i < argSize; i ++ )
                {
                    // Look for the short option's name (a single char)
                    nameIter = m_nameHash.find( arg.at( i ) );

                    if ( m_nameHash.constEnd() == nameIter )
                    {
                        // Not found
                        m_unknownOptionNames.append( arg.at( i ) );
                    }
                    else
                    {
                        // Found!
                        m_optionNames.append( nameIter.key() );
                        optionOffset = *nameIter;

                        if ( ! m_optionList.at( optionOffset ).m_bHasArg )
                        {
                            // This option has no argument
                            m_optionArgsList[ optionOffset ].append( emptyString );
                        }
                        else
                        {
                            // This option has an argument
                            if ( argSize - 1 == i )
                            {
                                // No more string left - use next arg.
                                argumentsOffset ++;

                                if ( argumentsOffset < argumentsSize )
                                {
                                    m_optionArgsList[ optionOffset ].append( arguments.at( argumentsOffset ) );
                                }
                            }
                            else
                            {
                                // Use rest of string.
                                m_optionArgsList[ optionOffset ].append( arg.mid( i + 1 ) );
                            }

                            // Don't process the rest of this string.
                            break;
                        } // if option takes an argument
                    } // if short option name found
                } // for loop through short option chars
            } // if char at pos 1 is not '-' (ie: it's a short option)
            else
            {
                // Long option

                if ( 2 == argSize )
                {
                    // Double dash found
                    if ( m_bStopParsingAtDoubleDash )
                    {
                        bParsingStopped = true;
                    }
                    else
                    {
                        m_leftoverArgs.append( arg );
                    }
                }
                else
                {
                    assignOffset = arg.indexOf( assignChar, 2 );
                    name = arg.mid( 2, ( -1 == assignOffset ? argSize : assignOffset ) - 2 );

                    // Look for the long option's name
                    nameIter = m_nameHash.find( name );

                    if ( m_nameHash.constEnd() == nameIter )
                    {
                        // Not found
                        m_unknownOptionNames.append( name );

                        if ( !m_bRemoveUnknownLongNames )
                        {
                            m_leftoverArgs.append( arg );
                        }
                    }
                    else
                    {
                        // Found!
                        m_optionNames.append( nameIter.key() );
                        optionOffset = *nameIter;

                        if ( ! m_optionList.at( optionOffset ).m_bHasArg )
                        {
                            // This option has no argument
                            m_optionArgsList[ optionOffset ].append( emptyString );
                        }
                        else
                        {
                            // This option has an argument

                            if ( -1 == assignOffset )
                            {
                                // No assignment character present - use next arg.
                                argumentsOffset ++;

                                if ( argumentsOffset < argumentsSize )
                                {
                                    m_optionArgsList[ optionOffset ].append( arguments.at( argumentsOffset ) );
                                }
                            }
                            else
                            {
                                // Assignment char was present
                                if ( argSize - 1 == assignOffset )
                                {
                                    // No more chars after assignment char.
                                    m_optionArgsList[ optionOffset ].append( emptyString );
                                }
                                else
                                {
                                    // Use rest of string after assignment char.
                                    m_optionArgsList[ optionOffset ].append( arg.mid( assignOffset + 1 ) );
                                }
                            } // if assignment char present

                        } // if option takes an argument
                    } // if long option name found

                } // if not "--"

            } // if it's a long option
        }

        bParseOk = true;
    }
    catch ( std::bad_alloc & )
    {
    }

    return bParseOk;

} // CliParser::parse()


QString CliParser::getArgument( const QString & name ) const
{
    QString arg;  // start of with a null string
    QStringList argList = getArgumentList( name );

    if ( ! argList.isEmpty() )
    {
        arg = argList.last();
    }

    return arg;

} // CliParser::getArgument()


QStringList CliParser::getArgumentList( const QString & name ) const
{
    QStringList argList;
    const NameHash_t::const_iterator nameIter = m_nameHash.find( name );

    if ( m_nameHash.constEnd() != nameIter )
    {
        const NameHash_t::mapped_type optionOffset = *nameIter;

        if ( m_optionArgsList.size() > optionOffset )
        {
            argList = m_optionArgsList.at( optionOffset );
        }
    }

    return argList;

} // CliParser::getArgumentList()


QStringList CliParser::getLeftoverArgs() const
{
    return m_leftoverArgs;
}


QStringList CliParser::getOptionNames() const
{
    return m_optionNames;
}


QStringList CliParser::getUnknownOptionNames() const
{
    return m_unknownOptionNames;
}
