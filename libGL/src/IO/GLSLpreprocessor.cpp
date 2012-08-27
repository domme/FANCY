#include "GLSLpreprocessor.h"
#include "FileReader.h"
#include "PathService.h"


GLSLpreprocessor::GLSLpreprocessor()
{
	
}

GLSLpreprocessor::~GLSLpreprocessor()
{
	
}

GLSLpreprocessor& GLSLpreprocessor::getInstance()
{
	static GLSLpreprocessor instance;
	return instance;
}

void GLSLpreprocessor::preprocessShader( String& rFinalShaderString, const String& szPathRelative )
{
	std::vector<std::string> vLines;
	FileReader::ReadTextFileLines( szPathRelative, vLines );
	
	std::vector<std::string>::iterator iterIncludeLine = vLines.end();
	int iFoundPos = -1;
	int iFirstQuotationPos = -1;
	int iSecondQutationPos = -1;
	bool bFound = false;

	do
	{
		iterIncludeLine = vLines.end();
		uint uIdx = 0;
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
			FileReader::ReadTextFileLines( szSubStr, vInsertLines );

			if( vInsertLines.size() > 0 )
			{
				vLines.insert( iterIncludeLine, vInsertLines.begin(), vInsertLines.end() );
			}
		}

		rFinalShaderString.clear();
	
		for( uint uIdx = 0; uIdx < vLines.size(); ++uIdx )
		{
			rFinalShaderString += vLines[ uIdx ] + "\n";
		}

	} while( bFound );
	
}

