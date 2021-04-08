# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <math.h>
# include <cctype>
# include <iostream>
# include <sstream>
# include <iomanip>
# include <string>
# include <vector>

using namespace std;

struct  Token {
  string tokenName  ;
  int typeNum  ;
}; // TokenType


struct TokenTree {
  Token * leftToken  ;
  Token * rightToken ;
  TokenTree * leftNode  ;
  TokenTree * rightNode  ;
  TokenTree * backNode  ;
}; // TokenType


enum Type {
  INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, LEFTPAREN, RIGHTPAREN
}; // Type

enum Error {
  LEFTERROR, RIGHTERROR, CLOSEERROR, EOFERROR, NOERROR
}; // Error

vector<Token> gTokens;
TokenTree * gTreeRoot = NULL;
TokenTree * gCurrentNode = NULL ;

int gLine = 0 ;
int gColumn = 0 ;

bool gIsEnd = false;
int gAtomType ;

string gErrorMsgName = "\0" ;
int gErrorMsgType = NOERROR ;
int gErrorLine = 0 ;
int gErrorColumn = 0 ;

// ------------------Setting Function--------------------- //

void ClearInput() {
  while ( cin.get() != '\n' ) cin.get() ;
} // ClearInput()

void SetErrorMsg( int errorType, string errorToken ) {
  gErrorMsgType = errorType ;
  gErrorMsgName = errorToken ;
  gErrorLine = gLine ;
  gErrorColumn = gColumn ;
} // SetErrorMsg()

void GlobalVariableReset() {
  gTreeRoot = NULL ;
  gTokens.clear() ;
  gColumn = 0 ;
  gLine = 1 ;
  gErrorLine = 0;
  gErrorColumn = 0;
} // GlobalVariableReset()

bool ExitDetect() {
  int leftparen = -1 ;
  int exit = -1 ;
  int rightparen = -1 ;
  string tokenString = "\0" ;
  
  for ( int i = 0 ; i < gTokens.size() ; i++ )
    tokenString += gTokens[i].tokenName ;
  
  leftparen = ( int ) tokenString.find( "(" ) ;
  exit = ( int ) tokenString.find( "exit" ) ;
  rightparen = ( int ) tokenString.find( ")" ) ;
  
  if ( exit != -1 && leftparen != -1 && rightparen != -1 && exit > leftparen && rightparen > exit ) {
    gIsEnd = true ;
    return true ;
  } // if

  
  return false ;
} // ExitDetect()

// ------------------Get Token--------------------- //
  
string StringProcess() {
  string inputString = "\0" ;
  bool closeQuote = false ;
  char ch = cin.get() ;
  gColumn ++ ;
  inputString += ch ;
  char peek = cin.peek() ;
  gAtomType = STRING ;
  
  while ( closeQuote == false && peek != '\n' && peek != EOF ) {
    
    ch = cin.get() ;
    gColumn ++ ;
    peek = cin.peek() ;
    
    if ( ch == '"' ) closeQuote = true ;
    
    if ( ch == '\\' && peek == '\\' ) {              // double slash
      cin.get();
      gColumn ++ ;
      inputString += '\\' ;
    } // if
    
    else if ( ch == '\\' && peek == 'n' ) {           // \n case
      cin.get() ;
      gColumn ++ ;
      inputString += '\n' ;
    } // if
    
    else if ( ch == '\\' && peek == 't' ) {          // \t case
      cin.get() ;
      gColumn ++ ;
      inputString += '\t' ;
    } // if
    
    else if ( ch == '\\' && peek == '"' ) {          // \" case
      cin.get() ;
      gColumn ++ ;
      inputString += '"' ;
    } // if
    
    else inputString += ch ;
    
  } // while
  
  if ( closeQuote == false ) {
    if ( peek != EOF )
      cin.get() ;
    gColumn ++ ;
    SetErrorMsg( CLOSEERROR, "\"" ) ;
    return "\0" ;
  } // if                                        // no closing quote
  
  return inputString ;
} // StringProcess()

string NumProcess( string atomExp ) {
  int floatLength = 0 ;
  int intLength = 0 ;
  int float4 = 0 ;
  if ( atomExp[0] == '+' ) atomExp = atomExp.substr( 1, atomExp.length() - 1 ) ;  // '+' case
  if ( atomExp[0] == '.'  ) atomExp = "0" + atomExp ;                             // .xxx case
  if ( atomExp[0] == '-' && atomExp[1] == '.' ) atomExp.insert( 1, "0" ) ;        // -.xxx case
  
  int dot = ( int ) atomExp.find( '.' );
  if ( dot != atomExp.npos ) {
    gAtomType = FLOAT ;
    floatLength = ( int ) ( atomExp.length() - 1 ) - dot ;
    intLength = ( int ) ( atomExp.length() - 1 ) - floatLength ;
    
    if ( floatLength > 3 ) {
      if ( ( int ) ( atomExp[intLength + 4] ) > 52 ) {
        float4 = ( ( int ) ( atomExp[intLength + 3] ) ) + 1 ;
        atomExp = atomExp.substr( 0, dot + 3 ) + ( char ) float4 ;
      } // if
      
      else atomExp = atomExp.substr( 0, dot + 4 ) ;                // 四捨五入
    } // if                                                        // float > 3
    
    else if ( floatLength < 3 ) {
      for ( int i = 0 ; i < ( 3 - floatLength ) ; i++ )
        atomExp = atomExp + "0" ;
    } // if                                                        // float < 3
  } // if                                                          // float case
  
  
  else gAtomType = INT ;
  return atomExp ;
} // NumProcess()

string AtomAnalyze( string atomExp ) {
  bool isNum = false ;
  bool isSymbol = false ;
  int signBit = 0 ;
  int digitBit = 0 ;
  int dotBit = 0;
  if ( atomExp == "#t" || atomExp == "t" ) {
    atomExp = "#t" ;
    gAtomType = T ;
  } // if
  
  else if ( atomExp == "nil" || atomExp == "#f" ) {
    atomExp = "nil" ;
    gAtomType = NIL ;
  } // if
  else if ( atomExp == "'"  ) {
    atomExp = "'" ;
    gAtomType = QUOTE ;
  } // if
  else if ( atomExp == "." ) {
    atomExp = "." ;
    gAtomType = DOT ;
  } // if
  
  else {
    
    for ( int i = 0 ; i < atomExp.length() ; i ++ ) {
      if ( ( ( int ) atomExp[i] >= 48 && ( int ) atomExp[i] <= 57 ) ||
           atomExp[i] == '+' || atomExp[i] == '-' ||  atomExp[i] == '.' ) {
        if ( atomExp[i] == '+' || atomExp[i] == '-' ) signBit ++ ;
        if ( ( int ) atomExp[i] >= 48 && ( int ) atomExp[i] <= 57 ) digitBit ++ ;
        if ( atomExp[i] == '.' ) dotBit ++ ;
        isNum = true;
      } // if
      
      else isSymbol = true;
    } // for                                                           // Num Judge
    
    if ( signBit > 1 || digitBit == 0 || dotBit > 1 ) isNum = false ;
    
    gAtomType = SYMBOL ;
    if ( isNum && !isSymbol ) atomExp = NumProcess( atomExp );
  } // else

  
  return  atomExp;
} // AtomAnalyze()

string GetAtom( ) {
  string atomExp = "\0" ;
  char ch = '\0' ;
  char peek = cin.peek() ;
  while ( peek != '\n' &&
          peek != ' ' && peek != ';' &&
          peek != '(' && peek != ')' && peek != '"' &&
          peek != '\0' && peek != EOF ) {
    ch = cin.get() ;
    gColumn ++ ;
    atomExp = atomExp + ch;
    peek = cin.peek() ;
  } // while
  
  atomExp = AtomAnalyze( atomExp );
  
  return atomExp ;
} // GetAtom()

char GetChar( ) {
  char peek = '\0' ;
  peek = cin.peek() ;
  while ( peek == ' ' || peek == '\t' || peek == '\n' || peek == ';'  ) {
    if ( peek == ';' ) {
      while ( peek != '\n' ) {
        cin.get() ;
        gColumn ++ ;
        peek = cin.peek() ;
      } // while
    } // if
    
    else if ( peek == '\n' ) {
      cin.get() ;
      gLine++ ;
      gColumn = 0 ;
    } // if
    
    else if ( peek == ' ' || peek == '\t' ) {
      cin.get() ;
      gColumn ++ ;
    } // if
    
    
    peek = cin.peek() ;
  } // while
  
  return peek ;
} // GetChar()

bool GetToken() {
  Token token;
  char peek = GetChar() ;

  
  if ( peek == '\0' || peek == EOF ) {
    gIsEnd = true ;
    return false;
  } // if
  
  else {
    
    if ( peek == '(' ) {
      token.tokenName = cin.get() ;
      gColumn ++ ;
      
      peek = GetChar() ;
      
      if ( peek == ')' ) {
        token.tokenName = "nil" ;
        cin.get() ;
        gColumn ++ ;
        gAtomType = NIL ;
      } // if                                                   // () case
      
      else
        gAtomType = LEFTPAREN;
    } // if                                         // left praen
    
    else if ( peek == ')' ) {
      token.tokenName = cin.get() ;
      gColumn ++ ;
      gAtomType = RIGHTPAREN;
    } // if             // right paren
    
    else if ( peek == '"' ) {
      token.tokenName = StringProcess() ;
      if ( token.tokenName == "\0" ) return false ;
    } // if                                        // string
    
    else token.tokenName = GetAtom();              // symbol
    
    token.typeNum = gAtomType ;
    gTokens.push_back( token ) ;
    
    return true;
  } // else
  
} // GetToken()

// ------------------Tree Build--------------------- //

bool IsAtom( int typeNum ) {
  if ( typeNum == SYMBOL || typeNum == INT ||
       typeNum == FLOAT || typeNum == STRING ||
       typeNum == NIL || typeNum == T )
    return true ;
  
  else return false ;
} // IsAtom()

void InitialNode() {
  gCurrentNode->leftNode = NULL ;
  gCurrentNode->leftToken = NULL ;
  gCurrentNode->rightNode = NULL ;
  gCurrentNode->rightToken = NULL ;
} // InitialNode()

void InsertAtomToTree() {
  if ( gTreeRoot != NULL ) {
    if ( gCurrentNode->leftToken == NULL ) {                              // left token null
      if ( gCurrentNode->leftNode == NULL ) {                             // left node null -> insert left
        gCurrentNode->leftToken = new Token ;
        gCurrentNode->leftToken->tokenName = gTokens.back().tokenName ;
        gCurrentNode->leftToken->typeNum = gTokens.back().typeNum ;
      } // if
      
      else if ( gCurrentNode->leftNode != NULL ) {                        // left node !null
        while ( gCurrentNode->rightNode != NULL )                         // find right node null
          gCurrentNode = gCurrentNode->backNode ;                         // and insert right token
        
        gCurrentNode->rightToken = new Token ;
        gCurrentNode->rightToken->tokenName = gTokens.back().tokenName ;
        gCurrentNode->rightToken->typeNum = gTokens.back().typeNum ;
      } // if
    } // if

    else if ( gCurrentNode->leftToken != NULL ) {                       // left token !null
      while ( gCurrentNode->rightToken != NULL )                        // find right node null
        gCurrentNode = gCurrentNode->backNode ;
      
      if ( gTokens[gTokens.size()-2].typeNum != DOT ) {                 // if !dot-> create right node
        gCurrentNode->rightNode = new TokenTree ;                       // and insert to left token
        gCurrentNode->rightNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->rightNode ;
        InitialNode();
        gCurrentNode->leftToken = new Token ;
        gCurrentNode->leftToken->tokenName = gTokens.back().tokenName ;
        gCurrentNode->leftToken->typeNum = gTokens.back().typeNum ;
      } // if
      
      else if ( gTokens[gTokens.size()-2].typeNum == DOT ) {            // if == dot-> insert right token
        gCurrentNode->rightToken = new Token ;
        gCurrentNode->rightToken->tokenName = gTokens.back().tokenName ;
        gCurrentNode->rightToken->typeNum = gTokens.back().typeNum ;
      } // if
    } // if
  } // if
} // InsertAtomToTree()

void BuildTree() {
  if ( gTreeRoot == NULL ) {
    gTreeRoot = new TokenTree ;
    gCurrentNode = gTreeRoot ;
    InitialNode();
  } // if
  
  else {
    if ( gCurrentNode->leftToken == NULL ) {                // left token null
      if ( gCurrentNode->leftNode == NULL ) {               // left node null-> create node
        gCurrentNode->leftNode = new TokenTree ;
        gCurrentNode->leftNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->leftNode ;
        InitialNode();
      } // if
      
      else if ( gCurrentNode->leftNode != NULL ) {          // left node !null
        while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
          gCurrentNode = gCurrentNode->backNode ;
        
        gCurrentNode->rightNode = new TokenTree ;
        gCurrentNode->rightNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->rightNode ;
        InitialNode();
      } // if
    } // if
    
    else if ( gCurrentNode->leftToken != NULL ) {         // left token !null
      while ( gCurrentNode->rightNode != NULL )           // find right node null-> create node
        gCurrentNode = gCurrentNode->backNode ;
      
      if ( gTokens[gTokens.size()-2].typeNum != DOT ) {                 // if !dot-> create right node
        gCurrentNode->rightNode = new TokenTree ;                       // and create left node
        gCurrentNode->rightNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->rightNode ;
        InitialNode();
        gCurrentNode->leftNode = new TokenTree ;
        gCurrentNode->leftNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->leftNode ;
        InitialNode();
      } // if
      
      else if ( gTokens[gTokens.size()-2].typeNum == DOT ) {            // if == dot-> build right node
        gCurrentNode->rightNode = new TokenTree ;
        gCurrentNode->rightNode->backNode = gCurrentNode;
        gCurrentNode = gCurrentNode->rightNode ;
        InitialNode();
      } // if
    } // if
  } // else
  

} // BuildTree()


bool SyntaxChecker() {
  if ( IsAtom( gTokens.back().typeNum ) ) {
    // cout << "Atom " ;
    
    if ( gTreeRoot != NULL ) InsertAtomToTree();
    return true ;
  } // if
  
  
  else if ( gTokens.back().typeNum == LEFTPAREN ) {
    // cout << "Left " ;
    if ( !GetToken() ) {
      SetErrorMsg( CLOSEERROR, gTokens.back().tokenName ) ;
      return false ;
    } // if
    
    BuildTree() ;                        // create node
    
    while ( SyntaxChecker() ) {
      if ( !GetToken() ) {
        SetErrorMsg( CLOSEERROR, gTokens.back().tokenName ) ;
        return false ;
      } // if
    } // while
    
    if ( gErrorMsgType == LEFTERROR ) return false;
    
    if ( gTokens.back().typeNum == DOT ) {
      // cout << "Dot " ;
      if ( GetToken() ) {
        if ( SyntaxChecker() ) {
          if ( !GetToken() ) {
            SetErrorMsg( CLOSEERROR, gTokens.back().tokenName ) ;
            return false ;
          } // if  no token
        } // if  syntax checker
        
        else {
          SetErrorMsg( LEFTERROR, gTokens.back().tokenName ) ;
          return false ;
        } // else
      } // if
      
      else {
        SetErrorMsg( CLOSEERROR, gTokens.back().tokenName ) ;
        return false ;
      } // else
    } // if
    
    if ( gTokens.back().typeNum == RIGHTPAREN ) {
      gCurrentNode = gCurrentNode->backNode ;
      return true;
    } // if
    else {
      SetErrorMsg( RIGHTERROR, gTokens.back().tokenName ) ;
      return false;
    } // else
    
  } // if
  
  else if ( gTokens.back().typeNum == QUOTE ) {
    // cout << "Quote " ;
    if ( GetToken() ) {
      if ( SyntaxChecker() ) return true ;
      else {
        SetErrorMsg( LEFTERROR, gTokens.back().tokenName ) ;
        return false ;
      } // else
    } // if
    
    else {
      SetErrorMsg( LEFTERROR, gTokens.back().tokenName ) ;
      return false ;
    } // else
  } // if
  
  if ( gTokens.size() > 2 ) {
    if ( !IsAtom( gTokens[gTokens.size()-2].typeNum ) && gTokens[gTokens.size()-2].typeNum != RIGHTPAREN )
      SetErrorMsg( LEFTERROR, gTokens.back().tokenName ) ;
  } // if
  
  else SetErrorMsg( LEFTERROR, gTokens.back().tokenName ) ;
  return false ;
  
} // SyntaxChecker()

// ------------------Print Function--------------------- //

void PrintSExp() {
  int printParenNum = 0 ;
  Token temp ;
  bool endLine = false ;
  vector<string> printVector ;
  
  for ( int i = 0 ; i < gTokens.size() ; i++ ) {
    if ( gTokens.size() == 1 ) cout << gTokens.back().tokenName << endl;
    else {
      
      if ( gTokens[i].typeNum == QUOTE ) {
        cout << "( " << "quote " ;
        temp.tokenName = ")" ;
        temp.typeNum = QUOTE ;
        gTokens.push_back( temp ) ;                                         // quote case
      } // if
      
      else if ( gTokens[i].typeNum == LEFTPAREN ) {
        if ( i != 0 ) {
          if ( gTokens[i-1].typeNum != DOT ) {
            cout << gTokens[i].tokenName << " " ;
            printParenNum ++ ;
          } // if
        } // if
        
        else {
          cout << gTokens[i].tokenName << " " ;
          printParenNum ++ ;
        } // else
      } // if
      
      else if ( gTokens[i].typeNum == RIGHTPAREN ) {
        if ( printParenNum != 0 ) {
          printParenNum -- ;
          for ( int i = 0 ; i < printParenNum ; i++ )
            cout << "  " ;
          cout << gTokens[i].tokenName << endl ;
          endLine = true ;
        } // if
      } // if
      
      else if ( gTokens[i].typeNum == DOT ) {
        if ( IsAtom( gTokens[i-1].typeNum ) && IsAtom( gTokens[i+1].typeNum ) &&
             gTokens[i+1].typeNum != NIL ) {
          cout << gTokens[i].tokenName << endl ;
          endLine = true ;
        } // if
      } // if
      
      else if ( gTokens[i].typeNum == NIL ) {
        if ( gTokens[i-1].typeNum != DOT  ) {
          cout << gTokens[i].tokenName << endl ;
          endLine = true ;
        } // if
      } // if
      
      else if ( IsAtom( gTokens[i].typeNum ) ) {
        cout << gTokens[i].tokenName << endl ;
        endLine = true ;
      } // if
      
      if ( endLine ) {
        if ( gTokens[i+1].typeNum != RIGHTPAREN ) {
          if ( gTokens[i+1].typeNum != DOT || gTokens[i+2].typeNum != NIL ) {
            for ( int i = 0 ; i < printParenNum ; i++ )
              cout << "  " ;
          } // if
        } // if
        
        endLine = false ;
      } // if
      
    } // else
  } // for
  
  cout << endl;
} // PrintSExp()


void PrintErrorMessage() {
  if ( gErrorMsgType == LEFTERROR )
    cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
    << gErrorLine << " Column " << gErrorColumn << " is >>" << gErrorMsgName << "<< " << endl;
  else if ( gErrorMsgType == RIGHTERROR )
    cout << "ERROR (unexpected token) : ')' expected when token at Line "
    << gErrorLine << " Column " << gErrorColumn << " is >>" << gErrorMsgName << "<< " << endl;
  else if ( gErrorMsgType == CLOSEERROR )
    cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line "
    << gErrorLine << " Column " << gErrorColumn << endl;
  else if ( gErrorMsgType == EOFERROR )
    cout << "ERROR (no more input) : END-OF-FILE encountered" ;
} // PrintErrorMessage()
  

// ------------------Main Function--------------------- //

int main() {
  cout << "Welcome to OurScheme!" << endl << endl ;
  int uTestNum = 0 ;
  cin >> uTestNum ;
  do {
    cout << "> " ;
    if ( GetToken() ) {
      if ( SyntaxChecker() && !ExitDetect() ) PrintSExp() ;
      else {
        PrintErrorMessage() ;
        ClearInput() ;
      } // else
    } // if
    
    else {
      if ( gErrorMsgType == NOERROR )
        SetErrorMsg( EOFERROR, " " ) ;
      PrintErrorMessage() ;
    } // else
    
    
    // cout << "(" << gColumn << " , " << gLine << ")" << endl;
    
    GlobalVariableReset() ;
  } while ( !gIsEnd );

  
  cout << endl << "Thanks for using OurScheme!" << "\n" ;
  return 0;
  
} // main()
