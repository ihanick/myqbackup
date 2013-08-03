/*!
 * \file
 * C++ interface for the \c CliParser class.
 *
 * \author Stephen Bryant <steve@bawue.de>
 * \date $Date: 2010-10-04 10:22:28 +0200 (Mon, 04 Oct 2010) $
 */

#ifndef CliParser_H
#define CliParser_H


#include <QString>
#include <QStringList>
#include <QHash>
#include <QList>

#include "clioption.h"


/*!
 * \class CliParser cliparser.h
 * \brief A simple parser of options on the command line interface.
 * \author Stephen Bryant <steve@bawue.de>
 *
 * This parser finds options and their values on the command line.  The parser
 * allows for long and short names, aliases (more than one name for the same
 * option) and option arguments.
 *
 * It can parse the options passed to the program or an arbitrary list of
 * options.  It can subsequently return argument values it found, or a list
 * of arguments that were left over.  The parser can also optionally remove
 * unrecognised options from the leftover list.
 *
 * Options on the command line are recognised as starting with a dash character.
 * The option "-" (single dash) is a special case, and is not treated as an option.
 * By default, the parser will stop parsing once the option "--" (double dash)
 * is encountered, although this behaviour can be changed.
 *
 * Short options are single letters.  The option "f" would be specified by
 * passing "-f" on the command line.  Short options can be be bundled - any
 * short option that does not take an argument can be immediately followed
 * by another short option.  For example, if "f" has no argument, it can be
 * bundled with option "b" like this: "-fb".
 *
 * Long options are more than one letter long.  The long option "foo" would
 * be passed as "--foo".  Long options can not be bundled together.
 *
 * Short options that take an argument will use the remaining characters in
 * the same argument.  For example, if "b" takes an argument, passing "-fbabc"
 * will treat "abc" as b's argument.  If there are no more characters, the
 * next argument is used - even if it starts with a dash.
 *
 * Long options are similar, but require an assignment operator to mark the
 * end of the long name, such as shown here: "--bar=value".  If there is no
 * assignment operator, the next argument is used - even if it starts with a
 * dash.
 *
 * The parser does not support optional arguments - if an option is set to
 * require an argument, one must be present.  If such an option is placed
 * last and has no argument, the option will be treated as if it had not been
 * specified.
 *
 * The parser does not automatically support negating or disabling long options
 * by using the format "--disable-foo" or "--no-foo".  However, a caller could
 * make an option with "no-foo" as one of its names, and handle the case
 * explicitly themselves.
 *
 * All values of option arguments are string only.  It is up to the caller to
 * convert to other types.
 */

class CliParser
{
public:

    //! Default constructor.
    CliParser();


    /*!
     * Whether to stop parsing when the option '--' is encountered.
     *
     * Set to \c true by default.
     *
     * \sa parse( const QStringList & )
     */
    bool m_bStopParsingAtDoubleDash;

    /*!
     * Remove unrecognised long-name options from the leftover list.
     *
     * Set to \c true by default.  Short options are always removed.
     *
     * \sa getLeftoverArgs()
     */
    bool m_bRemoveUnknownLongNames;


    /*!
     * Add an option to look for when parsing.
     *
     * If the option contains no names or any name that is in use by a
     * previously added option, adding it will fail.  Adding the option may
     * also fail if memory cannot be allocated.
     *
     * There is currently a limit of 65535 options.  Subsequent additions will
     * fail.
     *
     * \param option the option to add.
     * \return whether the option could be added.
     */
    bool addOption( const CliOption & option );


    /*!
     * Parse the command line for options.
     *
     * The command line is obtained from the current \c QCoreApplication
     * instance - it will fail if this is not available.  The first argument
     * in the list is the program name and is skipped.
     *
     * This method calls <tt>parse( const QStringList & )</tt>.
     *
     * \return whether the parsing succeeded.
     * \sa parse( const QStringList & )
     * \sa QCoreApplication::instance()
     * \sa QCoreApplication::arguments()
     */
    bool parse();


    /*!
     * Parse the given arguments for options.
     *
     * Any results from a previous parse operation are removed.  If
     * \c m_bStopParsingAtDoubleDash is \c true the parser will not look for
     * further options once it encounters the option "--"; this does not
     * include when "--" follows an option that requires an argument.
     *
     * Options that were successfully recognised, and their arguments, are
     * removed from the input list.  If \c m_bRemoveUnknownLongNames is
     * \c true, unrecognised options are removed and placed into a list of
     * unknown option names.  Anything left over is placed into a list of
     * leftover arguments.
     *
     * A long option that does not take an argument will still be recognised
     * if encountered in the form "--foo=value".  In this case, the argument
     * value will be ignored.
     *
     * \param arguments the list of arguments to parse.
     * \return whether the parsing succeeded.
     */
    bool parse( const QStringList & arguments );


    /*!
     * Get the argument for a given option.
     *
     * The name provided can be any long or short name of any option that was
     * added with \c addOption().  All of an option's aliases are treated as
     * being equivalent.  If the name is not recognised or that option was not
     * present, a null string is returned.
     *
     * For options found by the parser, an empty string is returned if the
     * option does not take an argument, otherwise the last argument found for
     * that option is returned.
     *
     * \param name the name of the option to look for.
     * \return a null string if not found, or a string representing the last
     * value found for the option.
     */
    QString getArgument( const QString & name ) const;


    /*!
     * Get a list of arguments for a given option.
     *
     * The name provided can be any long or short name of any option that was
     * added with \c addOption().  All of an option's aliases are treated as
     * being equivalent.  If the name is not recognised or that option was not
     * present, a null string is returned.
     *
     * For options found by the parser, the list will contain an entry for
     * each time the option was encountered by the parser.  These entries
     * will always be an empty string for options that do not take an argument.
     * Options that do take an argument will have the list populated with the
     * argument values in the order they were found.
     *
     * \param name the name of the option to look for.
     * \return a list of the arguments found for the option.
     */
    QStringList getArgumentList( const QString & name ) const;


    /*!
     * Get a list of left over arguments.
     *
     * These are all of the arguments that were not recognised as part of an
     * option.  If \c m_bRemoveUnknownLongNames is \c true, unrecognised
     * options will also have been removed.  Options with
     *
     * \return a list of left over arguments.
     */
    QStringList getLeftoverArgs() const;


    /*!
     * Get a list of option names that were found.
     *
     * This returns a list of all the recognised option names found by the
     * parser, in the order in which they were found.  For any long options
     * that were in the form "--foo=value", the value part will have been
     * dropped.
     *
     * The names in this list do not include the preceding dash characters.
     * Names may appear more than once in this list if they were encountered
     * more than once by the parser.
     *
     * Any entry in the list can be used with \c getArgument() or with
     * \c getArgumentList() to get any relevant arguments.
     *
     * \return a list of option names found by the parser.
     */
    QStringList getOptionNames() const;


    /*!
     * Get a list of unknown option names.
     *
     * This list will include both long an short name options that were not
     * recognised.  For any long options that were in the form "--foo=value",
     * the value part will have been dropped and only the long name is added.
     *
     * The names in this list do not include the preceding dash characters.
     * Names may appear more than once in this list if they were encountered
     * more than once by the parser.
     *
     * \return a list of unknown option names.
     */
    QStringList getUnknownOptionNames() const;


private:

    //! List of options added by addOption().
    QList< CliOption > m_optionList;

    //! List of arguments found for each option.
    QList< QStringList > m_optionArgsList;


    typedef QHash< QString, quint16 > NameHash_t;

    //! Hash mapping option names to their offsets in m_optionList and m_optionArgsList.
    NameHash_t m_nameHash;

    // The maximum number of options allowed.
    static const NameHash_t::mapped_type m_maxOptionCount = ~ 0;

    //! Arguments which did not belong to any option.
    QStringList m_leftoverArgs;

    //! Names of options found.
    QStringList m_optionNames;

    //! Names of options which were unknown.
    QStringList m_unknownOptionNames;

};

#endif // CliParser_H
