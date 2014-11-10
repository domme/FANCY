#include "GLSLpreprocessor.h"
#include "FileReader.h"
#include "PathService.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  GLSLpreprocessor::GLSLpreprocessor()
  {
  
  }
//---------------------------------------------------------------------------//
  GLSLpreprocessor::~GLSLpreprocessor()
  {
  
  }
//---------------------------------------------------------------------------//
  void GLSLpreprocessor::preprocessShader( String& rFinalShaderString, const String& szPathRelative )
  {
    std::vector<std::string> vLines;
    Fancy::IO::FileReader::ReadTextFileLines( szPathRelative, vLines );
  
    std::vector<std::string>::iterator iterIncludeLine = vLines.end();
    int iFoundPos = -1;
    int iFirstQuotationPos = -1;
    int iSecondQutationPos = -1;
    bool bFound = false;

    do
    {
      iterIncludeLine = vLines.end();
      uint32 uIdx = 0;
      for( std::vector<std::string>::iterator iter = vLines.begin(); iter != vLines.end(); ++iter )
      {
        iFoundPos = iter->find( "#include" );
        if( iFoundPos != String::npos ) //#include found!
        {
          iFirstQuotationPos = iter->find_first_of( "\"" );
          iSecondQutationPos = iter->find_last_of( "\"" );
          iterIncludeLine = iter;
          break;
        }
        uIdx++;
      }

      if( iterIncludeLine != vLines.end() && iFoundPos > -1 && iFirstQuotationPos > -1 && iSecondQutationPos > -1 && ( iSecondQutationPos > iFirstQuotationPos ) )
      {
        bFound = true;
      }

      else
      {
        bFound = false;
      }

      if( bFound )
      {
        String szSubStr = iterIncludeLine->substr( iFirstQuotationPos + 1, iSecondQutationPos - ( iFirstQuotationPos + 1 ) );
        //clear the "#include..." line
        vLines[ uIdx ] = "";

        szSubStr = "shader/" + szSubStr;

        std::vector<std::string> vInsertLines;
        Fancy::IO::FileReader::ReadTextFileLines( szSubStr, vInsertLines );

        if( vInsertLines.size() > 0 )
        {
          vLines.insert( iterIncludeLine, vInsertLines.begin(), vInsertLines.end() );
        }
      }

      rFinalShaderString.clear();
  
      for( uint32 uIdx = 0; uIdx < vLines.size(); ++uIdx )
      {
        rFinalShaderString += vLines[ uIdx ] + "\n";
      }

    } while( bFound );
  
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4