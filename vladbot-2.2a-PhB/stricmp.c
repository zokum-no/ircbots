
int strnicmp( const char *src, const char *dest, int length )
{
  char srcCh, destCh;
  
  while( length-- )
    {
      
      /* Need to be careful with toupper() side-effects */
      srcCh = *src++;
      srcCh = toupper( srcCh );
      destCh = *dest++;
      destCh = toupper( destCh );
      
      if( srcCh != destCh )
	return( srcCh - destCh );
    }
  
  return( 0 );
}

int stricmp( const char *src, const char *dest )
{
  int srcLen = strlen( src ), destLen = strlen( dest );
  
  if( srcLen != destLen )
    return( srcLen - destLen );
  return( strnicmp( src, dest, ( srcLen > destLen ) ? srcLen : destLen ) );
}
