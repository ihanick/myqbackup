/*!
 * \file
 * C++ implementation for the \c CliOption class.
 *
 * \author Stephen Bryant <steve@bawue.de>
 * \date $Date: 2010-09-23 17:59:55 +0200 (Thu, 23 Sep 2010) $
 */

#include "clioption.h"

#include <QLatin1Char>


CliOption::CliOption()
    : m_bHasArg( false )
{
}


CliOption::CliOption( const QStringList & names, const bool bHasArg )
    : m_bHasArg( bHasArg )
{
    setNames( names );
}


void CliOption::setNames( const QStringList & names )
{
    QStringList::const_iterator iter;
    static const QLatin1Char dashChar( '-' );

    for ( iter =  names.constBegin();
          iter != names.constEnd();
          iter ++ )
    {
        if ( ! (*iter).isEmpty() &&
             ( (*iter).at( 0 ) != dashChar ) )
        {
            if ( (*iter).size() == 1 )
            {
                m_shortnameSet.insert( (*iter).at( 0 ) );
            }
            else
            {
                m_longnameSet.insert( *iter );
            }
        }
    } // for loop through names
} // CliOption::setNames()
