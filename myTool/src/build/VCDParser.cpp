// A Bison parser, made by GNU Bison 3.5.1.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.


// Take the name prefix into account.
#define yylex   VCDParserlex



#include "VCDParser.hpp"


// Unqualified %code blocks.
#line 36 "./src/VCDParser.ypp"


#include "VCDFileParser.hpp"

//! Current time while parsing the VCD file.
VCDTime current_time = 0;


#line 56 "./build/VCDParser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace VCDParser {
#line 147 "./build/VCDParser.cpp"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (VCDFileParser & driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_number_type
  parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 32: // TOK_VALUE
        value.YY_MOVE_OR_COPY< VCDBit > (YY_MOVE (that.value));
        break;

      case 23: // TOK_KW_BEGIN
      case 24: // TOK_KW_FORK
      case 25: // TOK_KW_FUNCTION
      case 26: // TOK_KW_MODULE
      case 27: // TOK_KW_TASK
      case 45: // scope_type
        value.YY_MOVE_OR_COPY< VCDScopeType > (YY_MOVE (that.value));
        break;

      case 51: // reference
        value.YY_MOVE_OR_COPY< VCDSignal* > (YY_MOVE (that.value));
        break;

      case 28: // TOK_TIME_NUMBER
        value.YY_MOVE_OR_COPY< VCDTimeRes > (YY_MOVE (that.value));
        break;

      case 29: // TOK_TIME_UNIT
        value.YY_MOVE_OR_COPY< VCDTimeUnit > (YY_MOVE (that.value));
        break;

      case 30: // TOK_VAR_TYPE
        value.YY_MOVE_OR_COPY< VCDVarType > (YY_MOVE (that.value));
        break;

      case 38: // TOK_DECIMAL_NUM
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case 9: // TOK_COMMENT_TEXT
      case 11: // TOK_DATE_TEXT
      case 18: // TOK_VERSION_TEXT
      case 33: // TOK_BIN_NUM
      case 35: // TOK_REAL_NUM
      case 37: // TOK_IDENTIFIER
      case 52: // comment_text
      case 53: // version_text
      case 54: // date_text
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 32: // TOK_VALUE
        value.move< VCDBit > (YY_MOVE (that.value));
        break;

      case 23: // TOK_KW_BEGIN
      case 24: // TOK_KW_FORK
      case 25: // TOK_KW_FUNCTION
      case 26: // TOK_KW_MODULE
      case 27: // TOK_KW_TASK
      case 45: // scope_type
        value.move< VCDScopeType > (YY_MOVE (that.value));
        break;

      case 51: // reference
        value.move< VCDSignal* > (YY_MOVE (that.value));
        break;

      case 28: // TOK_TIME_NUMBER
        value.move< VCDTimeRes > (YY_MOVE (that.value));
        break;

      case 29: // TOK_TIME_UNIT
        value.move< VCDTimeUnit > (YY_MOVE (that.value));
        break;

      case 30: // TOK_VAR_TYPE
        value.move< VCDVarType > (YY_MOVE (that.value));
        break;

      case 38: // TOK_DECIMAL_NUM
        value.move< int > (YY_MOVE (that.value));
        break;

      case 9: // TOK_COMMENT_TEXT
      case 11: // TOK_DATE_TEXT
      case 18: // TOK_VERSION_TEXT
      case 33: // TOK_BIN_NUM
      case 35: // TOK_REAL_NUM
      case 37: // TOK_IDENTIFIER
      case 52: // comment_text
      case 53: // version_text
      case 54: // date_text
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 32: // TOK_VALUE
        value.copy< VCDBit > (that.value);
        break;

      case 23: // TOK_KW_BEGIN
      case 24: // TOK_KW_FORK
      case 25: // TOK_KW_FUNCTION
      case 26: // TOK_KW_MODULE
      case 27: // TOK_KW_TASK
      case 45: // scope_type
        value.copy< VCDScopeType > (that.value);
        break;

      case 51: // reference
        value.copy< VCDSignal* > (that.value);
        break;

      case 28: // TOK_TIME_NUMBER
        value.copy< VCDTimeRes > (that.value);
        break;

      case 29: // TOK_TIME_UNIT
        value.copy< VCDTimeUnit > (that.value);
        break;

      case 30: // TOK_VAR_TYPE
        value.copy< VCDVarType > (that.value);
        break;

      case 38: // TOK_DECIMAL_NUM
        value.copy< int > (that.value);
        break;

      case 9: // TOK_COMMENT_TEXT
      case 11: // TOK_DATE_TEXT
      case 18: // TOK_VERSION_TEXT
      case 33: // TOK_BIN_NUM
      case 35: // TOK_REAL_NUM
      case 37: // TOK_IDENTIFIER
      case 52: // comment_text
      case 53: // version_text
      case 54: // date_text
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 32: // TOK_VALUE
        value.move< VCDBit > (that.value);
        break;

      case 23: // TOK_KW_BEGIN
      case 24: // TOK_KW_FORK
      case 25: // TOK_KW_FUNCTION
      case 26: // TOK_KW_MODULE
      case 27: // TOK_KW_TASK
      case 45: // scope_type
        value.move< VCDScopeType > (that.value);
        break;

      case 51: // reference
        value.move< VCDSignal* > (that.value);
        break;

      case 28: // TOK_TIME_NUMBER
        value.move< VCDTimeRes > (that.value);
        break;

      case 29: // TOK_TIME_UNIT
        value.move< VCDTimeUnit > (that.value);
        break;

      case 30: // TOK_VAR_TYPE
        value.move< VCDVarType > (that.value);
        break;

      case 38: // TOK_DECIMAL_NUM
        value.move< int > (that.value);
        break;

      case 9: // TOK_COMMENT_TEXT
      case 11: // TOK_DATE_TEXT
      case 18: // TOK_VERSION_TEXT
      case 33: // TOK_BIN_NUM
      case 35: // TOK_REAL_NUM
      case 37: // TOK_IDENTIFIER
      case 52: // comment_text
      case 53: // version_text
      case 54: // date_text
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
#line 29 "./src/VCDParser.ypp"
{
    yyla.location.begin.filename = yyla.location.end.filename = &driver.filepath;
}

#line 636 "./build/VCDParser.cpp"


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (driver));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case 32: // TOK_VALUE
        yylhs.value.emplace< VCDBit > ();
        break;

      case 23: // TOK_KW_BEGIN
      case 24: // TOK_KW_FORK
      case 25: // TOK_KW_FUNCTION
      case 26: // TOK_KW_MODULE
      case 27: // TOK_KW_TASK
      case 45: // scope_type
        yylhs.value.emplace< VCDScopeType > ();
        break;

      case 51: // reference
        yylhs.value.emplace< VCDSignal* > ();
        break;

      case 28: // TOK_TIME_NUMBER
        yylhs.value.emplace< VCDTimeRes > ();
        break;

      case 29: // TOK_TIME_UNIT
        yylhs.value.emplace< VCDTimeUnit > ();
        break;

      case 30: // TOK_VAR_TYPE
        yylhs.value.emplace< VCDVarType > ();
        break;

      case 38: // TOK_DECIMAL_NUM
        yylhs.value.emplace< int > ();
        break;

      case 9: // TOK_COMMENT_TEXT
      case 11: // TOK_DATE_TEXT
      case 18: // TOK_VERSION_TEXT
      case 33: // TOK_BIN_NUM
      case 35: // TOK_REAL_NUM
      case 37: // TOK_IDENTIFIER
      case 52: // comment_text
      case 53: // version_text
      case 54: // date_text
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 10:
#line 111 "./src/VCDParser.ypp"
                                                {
    driver.fh -> comment = yystack_[1].value.as < std::string > ();
}
#line 810 "./build/VCDParser.cpp"
    break;

  case 11:
#line 114 "./src/VCDParser.ypp"
                                                {
    driver.fh -> date = yystack_[1].value.as < std::string > ();
}
#line 818 "./build/VCDParser.cpp"
    break;

  case 13:
#line 118 "./src/VCDParser.ypp"
                                                         {
    // PUSH the current scope stack.
    
    VCDScope * new_scope = new VCDScope();
    new_scope -> name = yystack_[1].value.as < std::string > ();
    new_scope -> type = yystack_[2].value.as < VCDScopeType > ();
    new_scope -> parent = driver.scopes.top();
    
    driver.fh -> add_scope(
        new_scope
    );
    
    driver.scopes.top() -> children.push_back(new_scope);
    driver.scopes.push(new_scope);

}
#line 839 "./build/VCDParser.cpp"
    break;

  case 14:
#line 134 "./src/VCDParser.ypp"
                                          {
    // PUSH the current scope stack.
    
    VCDScope * new_scope = new VCDScope();
    new_scope -> name = "";
    new_scope -> type = yystack_[1].value.as < VCDScopeType > ();
    new_scope -> parent = driver.scopes.top();
    
    driver.fh -> add_scope(
        new_scope
    );
    
    driver.scopes.top() -> children.push_back(new_scope);
    driver.scopes.push(new_scope);

}
#line 860 "./build/VCDParser.cpp"
    break;

  case 15:
#line 150 "./src/VCDParser.ypp"
                                                              {
    driver.fh -> time_resolution = yystack_[2].value.as < VCDTimeRes > ();
    driver.fh -> time_units      = yystack_[1].value.as < VCDTimeUnit > ();
}
#line 869 "./build/VCDParser.cpp"
    break;

  case 16:
#line 154 "./src/VCDParser.ypp"
                               {
    // POP the current scope stack.

    driver.scopes.pop();

}
#line 880 "./build/VCDParser.cpp"
    break;

  case 17:
#line 161 "./src/VCDParser.ypp"
               {
    // Add this variable to the current scope.

    VCDSignal * new_signal  = yystack_[1].value.as < VCDSignal* > ();
    new_signal -> type      = yystack_[4].value.as < VCDVarType > ();
    new_signal -> size      = yystack_[3].value.as < int > ();
    new_signal -> hash      = yystack_[2].value.as < std::string > ();
    if (new_signal->size == 1) {
        //assert((new_signal->lindex == -1) || (new_signal->rindex == -1));
    } else {
        if(new_signal->lindex == -1){
            new_signal->lindex = new_signal -> size -1;
            new_signal->rindex = 0;
        }
        assert((new_signal->lindex > 0));
        assert((new_signal->lindex - new_signal->rindex + 1 == new_signal->size));
    }
    VCDScope * scope = driver.scopes.top();
    scope -> signals.push_back(new_signal);

    driver.fh -> add_signal(new_signal);

}
#line 908 "./build/VCDParser.cpp"
    break;

  case 18:
#line 184 "./src/VCDParser.ypp"
                                            {
    driver.fh -> version = yystack_[1].value.as < std::string > ();
}
#line 916 "./build/VCDParser.cpp"
    break;

  case 26:
#line 200 "./src/VCDParser.ypp"
    { yylhs.value.as < VCDScopeType > () = yystack_[0].value.as < VCDScopeType > (); }
#line 922 "./build/VCDParser.cpp"
    break;

  case 27:
#line 201 "./src/VCDParser.ypp"
    { yylhs.value.as < VCDScopeType > () = yystack_[0].value.as < VCDScopeType > (); }
#line 928 "./build/VCDParser.cpp"
    break;

  case 28:
#line 202 "./src/VCDParser.ypp"
    { yylhs.value.as < VCDScopeType > () = yystack_[0].value.as < VCDScopeType > (); }
#line 934 "./build/VCDParser.cpp"
    break;

  case 29:
#line 203 "./src/VCDParser.ypp"
    { yylhs.value.as < VCDScopeType > () = yystack_[0].value.as < VCDScopeType > (); }
#line 940 "./build/VCDParser.cpp"
    break;

  case 30:
#line 204 "./src/VCDParser.ypp"
    { yylhs.value.as < VCDScopeType > () = yystack_[0].value.as < VCDScopeType > (); }
#line 946 "./build/VCDParser.cpp"
    break;

  case 31:
#line 207 "./src/VCDParser.ypp"
                                           {
    current_time =  yystack_[0].value.as < int > ();
    if (current_time > driver.end_time)
        YYACCEPT;
    if (current_time > driver.start_time)
        driver.fh    -> add_timestamp(yystack_[0].value.as < int > ());
}
#line 958 "./build/VCDParser.cpp"
    break;

  case 36:
#line 223 "./src/VCDParser.ypp"
                                               {

    VCDSignalHash   hash  = yystack_[0].value.as < std::string > ();
    if (current_time > driver.start_time) {
        VCDTimedValue * toadd = new VCDTimedValue();

        toadd -> time   = current_time;
        toadd -> value  = new VCDValue(yystack_[1].value.as < VCDBit > ());

        driver.fh -> add_signal_value(toadd, hash);
    }

}
#line 976 "./build/VCDParser.cpp"
    break;

  case 37:
#line 239 "./src/VCDParser.ypp"
                                   {

    VCDSignalHash   hash  = yystack_[0].value.as < std::string > ();
    VCDTimedValue * toadd = new VCDTimedValue();
    
    toadd -> time   = current_time;

    VCDBitVector * vec = new VCDBitVector();
    VCDValue * val = new VCDValue(vec);

    for(int i =1; i < yystack_[1].value.as < std::string > ().size(); i ++) {
        switch(yystack_[1].value.as < std::string > ()[i]) {
            case '0':
                vec -> push_back(VCD_0);
                break;
            case '1':
                vec -> push_back(VCD_1);
                break;
            case 'x':
            case 'X':
                vec -> push_back(VCD_X);
                break;
            case 'z':
            case 'Z':
                vec -> push_back(VCD_Z);
                break;
            default:
                vec -> push_back(VCD_X);
                break;
        }
    }

    toadd -> value = val;

    driver.fh -> add_signal_value(toadd, hash);

}
#line 1018 "./build/VCDParser.cpp"
    break;

  case 38:
#line 276 "./src/VCDParser.ypp"
                                   {

    VCDSignalHash   hash  = yystack_[0].value.as < std::string > ();
    VCDTimedValue * toadd = new VCDTimedValue();
    
    toadd -> time   = current_time;
    toadd -> value  = 0;

    VCDValue * val;
    VCDReal real_value;
    
    // Legal way of parsing dumped floats according to the spec.
    // Sec 21.7.2.1, paragraph 4.
    const char * buffer = yystack_[1].value.as < std::string > ().c_str() + 1;
    float tmp;
    std::sscanf(buffer, "%g", &tmp);
    real_value = tmp;
    
    toadd -> value = new VCDValue(real_value);
    driver.fh -> add_signal_value(toadd, hash);
}
#line 1044 "./build/VCDParser.cpp"
    break;

  case 39:
#line 299 "./src/VCDParser.ypp"
                   {
    yylhs.value.as < VCDSignal* > ()  = new VCDSignal();
    yylhs.value.as < VCDSignal* > () -> reference = yystack_[0].value.as < std::string > ();
    yylhs.value.as < VCDSignal* > () -> lindex = -1;
    yylhs.value.as < VCDSignal* > () -> rindex = -1;
}
#line 1055 "./build/VCDParser.cpp"
    break;

  case 40:
#line 305 "./src/VCDParser.ypp"
                                                              {
    yylhs.value.as < VCDSignal* > ()  = new VCDSignal();
    yylhs.value.as < VCDSignal* > () -> reference = yystack_[3].value.as < std::string > ();
    yylhs.value.as < VCDSignal* > () -> lindex = yystack_[1].value.as < int > ();
    yylhs.value.as < VCDSignal* > () -> rindex = yystack_[1].value.as < int > ();
}
#line 1066 "./build/VCDParser.cpp"
    break;

  case 41:
#line 312 "./src/VCDParser.ypp"
                 {
    yylhs.value.as < VCDSignal* > ()  = new VCDSignal();
    yylhs.value.as < VCDSignal* > () -> reference = yystack_[5].value.as < std::string > ();
    if (yystack_[3].value.as < int > () == 0 && yystack_[3].value.as < int > () == yystack_[1].value.as < int > ()) {
        yylhs.value.as < VCDSignal* > () -> lindex = -1;
        yylhs.value.as < VCDSignal* > () -> rindex = -1;
    } else {
        yylhs.value.as < VCDSignal* > () -> lindex = yystack_[3].value.as < int > ();
        yylhs.value.as < VCDSignal* > () -> rindex = yystack_[1].value.as < int > ();
    }
}
#line 1082 "./build/VCDParser.cpp"
    break;

  case 42:
#line 325 "./src/VCDParser.ypp"
            {
    yylhs.value.as < std::string > () = std::string();
}
#line 1090 "./build/VCDParser.cpp"
    break;

  case 43:
#line 328 "./src/VCDParser.ypp"
                     {
    yylhs.value.as < std::string > () = std::string(yystack_[0].value.as < std::string > ());
}
#line 1098 "./build/VCDParser.cpp"
    break;

  case 44:
#line 333 "./src/VCDParser.ypp"
          {
    yylhs.value.as < std::string > () = std::string();
}
#line 1106 "./build/VCDParser.cpp"
    break;

  case 45:
#line 336 "./src/VCDParser.ypp"
                     {
    yylhs.value.as < std::string > () = std::string(yystack_[0].value.as < std::string > ());
}
#line 1114 "./build/VCDParser.cpp"
    break;

  case 46:
#line 342 "./src/VCDParser.ypp"
          {
    yylhs.value.as < std::string > () = std::string();
}
#line 1122 "./build/VCDParser.cpp"
    break;

  case 47:
#line 345 "./src/VCDParser.ypp"
                  {
    yylhs.value.as < std::string > () = std::string(yystack_[0].value.as < std::string > ());
}
#line 1130 "./build/VCDParser.cpp"
    break;


#line 1134 "./build/VCDParser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[+yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yy_error_token_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yy_error_token_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    std::ptrdiff_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */
    if (!yyla.empty ())
      {
        symbol_number_type yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];

        int yyn = yypact_[+yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yy_error_token_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -29;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
      49,    43,    -2,    -1,   -10,    -8,    14,    -6,     4,   -28,
     -28,   -28,   -28,   -13,   -11,     2,    13,    41,    49,    66,
     -29,   -29,   -29,   -29,   -29,   -29,   -29,    -4,   -29,    35,
     -29,    36,   -29,   -29,   -29,   -29,   -29,   -29,    -5,    25,
     -29,    22,   -29,    60,     1,     5,    16,    23,   -29,   -29,
     -29,   -29,   -29,    66,   -29,   -28,   -29,   -29,   -29,   -29,
     -29,   -29,    65,    70,    42,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,    46,    90,    87,    57,   -29,     6,   -29,    58,
      69,   -29
  };

  const signed char
  parser::yydefact_[] =
  {
       2,    42,    46,     0,     0,     0,     0,     0,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     4,
       6,     8,    24,    25,    34,    35,    43,     0,    32,     0,
      47,     0,    12,    26,    27,    28,    29,    30,     0,     0,
      16,     0,    45,     0,     0,     0,     0,     0,    31,    36,
      37,    38,     1,     3,     7,     0,     9,    23,    33,    10,
      11,    14,     0,     0,     0,    18,    19,    20,    21,    22,
      13,    15,     0,    39,     0,     0,    17,     0,    40,     0,
       0,    41
  };

  const signed char
  parser::yypgoto_[] =
  {
     -29,   -29,   -29,    82,    84,   -18,   -29,   -29,    80,     0,
     -29,   -29,   -29,   -29,   -29,   -29
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,    17,    18,    19,    20,    21,    38,    22,    27,    28,
      24,    25,    74,    29,    43,    31
  };

  const signed char
  parser::yytable_[] =
  {
      23,    56,    61,    57,    14,    15,    32,    16,    66,    30,
      78,    79,    67,    33,    34,    35,    36,    37,    23,    23,
      39,    40,    42,    68,    41,    48,    49,    58,    14,    15,
      69,    16,    62,    14,    15,    56,    16,    14,    15,    50,
      16,    52,    59,    60,    58,    58,    58,    58,    14,    15,
      51,    16,    26,    23,    63,    14,    15,     1,    16,     2,
      64,     3,     4,     5,     6,     7,     8,    65,     9,    10,
      11,    12,    70,    81,    55,    14,    15,    71,    16,    72,
      13,    14,    15,    73,    16,     9,    10,    11,    12,    44,
      45,    46,    47,    75,    76,    77,    80,    13,    14,    15,
      53,    16,    54
  };

  const signed char
  parser::yycheck_[] =
  {
       0,    19,     7,     7,    32,    33,     7,    35,     7,    11,
       4,     5,     7,    23,    24,    25,    26,    27,    18,    19,
      28,     7,    18,     7,    30,    38,    37,    27,    32,    33,
       7,    35,    37,    32,    33,    53,    35,    32,    33,    37,
      35,     0,     7,     7,    44,    45,    46,    47,    32,    33,
      37,    35,     9,    53,    29,    32,    33,     8,    35,    10,
      38,    12,    13,    14,    15,    16,    17,     7,    19,    20,
      21,    22,     7,     4,     8,    32,    33,     7,    35,    37,
      31,    32,    33,    37,    35,    19,    20,    21,    22,     9,
      10,    11,    12,     3,     7,    38,    38,    31,    32,    33,
      18,    35,    18
  };

  const signed char
  parser::yystos_[] =
  {
       0,     8,    10,    12,    13,    14,    15,    16,    17,    19,
      20,    21,    22,    31,    32,    33,    35,    40,    41,    42,
      43,    44,    46,    48,    49,    50,     9,    47,    48,    52,
      11,    54,     7,    23,    24,    25,    26,    27,    45,    28,
       7,    30,    18,    53,    47,    47,    47,    47,    38,    37,
      37,    37,     0,    42,    43,     8,    44,     7,    48,     7,
       7,     7,    37,    29,    38,     7,     7,     7,     7,     7,
       7,     7,    37,    37,    51,     3,     7,    38,     4,     5,
      38,     4
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    39,    40,    40,    40,    40,    41,    41,    42,    42,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    44,
      44,    44,    44,    44,    44,    44,    45,    45,    45,    45,
      45,    46,    47,    47,    48,    48,    49,    50,    50,    51,
      51,    51,    52,    52,    53,    53,    54,    54
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     2,     1,     2,
       3,     3,     2,     4,     3,     4,     2,     6,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     2,     1,     1,     2,     2,     2,     1,
       4,     6,     0,     1,     0,     1,     0,     1
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "TOK_BRACKET_O",
  "TOK_BRACKET_C", "TOK_COLON", "TOK_DOLLAR", "TOK_KW_END",
  "TOK_KW_COMMENT", "TOK_COMMENT_TEXT", "TOK_KW_DATE", "TOK_DATE_TEXT",
  "TOK_KW_ENDDEFINITIONS", "TOK_KW_SCOPE", "TOK_KW_TIMESCALE",
  "TOK_KW_UPSCOPE", "TOK_KW_VAR", "TOK_KW_VERSION", "TOK_VERSION_TEXT",
  "TOK_KW_DUMPALL", "TOK_KW_DUMPOFF", "TOK_KW_DUMPON", "TOK_KW_DUMPVARS",
  "TOK_KW_BEGIN", "TOK_KW_FORK", "TOK_KW_FUNCTION", "TOK_KW_MODULE",
  "TOK_KW_TASK", "TOK_TIME_NUMBER", "TOK_TIME_UNIT", "TOK_VAR_TYPE",
  "TOK_HASH", "TOK_VALUE", "TOK_BIN_NUM", "TOK_BINARY_NUMBER",
  "TOK_REAL_NUM", "TOK_REAL_NUMBER", "TOK_IDENTIFIER", "TOK_DECIMAL_NUM",
  "$accept", "input", "declaration_commands", "simulation_commands",
  "declaration_command", "simulation_command", "scope_type",
  "simulation_time", "value_changes", "value_change",
  "scalar_value_change", "vector_value_change", "reference",
  "comment_text", "version_text", "date_text", YY_NULLPTR
  };

#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,    94,    94,    95,    96,    97,   101,   102,   106,   107,
     111,   114,   117,   118,   134,   150,   154,   160,   184,   190,
     191,   192,   193,   194,   195,   196,   200,   201,   202,   203,
     204,   207,   216,   217,   220,   221,   223,   239,   276,   299,
     305,   311,   325,   328,   333,   336,   342,   345
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // VCDParser
#line 1583 "./build/VCDParser.cpp"

#line 349 "./src/VCDParser.ypp"



void VCDParser::parser::error (
    const location_type& l,
    const std::string& m) {
    driver.error(l,m);
}
