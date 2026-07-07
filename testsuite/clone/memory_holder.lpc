//Globals
mapping gmap;
int *garray_int;
string *garray_str;
float *garray_float;
mixed *garray_mixed;
object *garray_object;

void populate_map( int flag )
{
  if( !flag )
  {
    mapping tmp_map = ([]);
    for( int i = 0 ; i < 10000 ; i ++ ) tmp_map[i]=i;
  }
  else
  {
    gmap = ([]);
    for( int i = 0 ; i < 10000 ; i ++ ) gmap[i]=i;
  }
}

void populate_int( int flag )
{
  if( !flag )
  {
    int *tmp_int;tmp_int = allocate( 10000 , (: $1 :) );
  }
  else garray_int = allocate( 10000 , (: $1 :) );
}

void populate_string( int flag )
{
  if( !flag )
  {
    string *tmp_string;tmp_string = allocate( 10000 , (: $1+"" :) );
  }
  else garray_str = allocate( 10000 , (: $1+"" :) );
}

void populate_float( int flag )
{
  if( !flag )
  {
    float *tmp_float;tmp_float = allocate( 10000 , (: $1*1.0:) );
  }
  else garray_float = allocate( 10000 , (: $1*1.0 :) );
}

void populate_mixed( int flag )
{
  if( !flag )
  {
    mixed *tmp_mixed=({});
    for( int i = 0 ; i < 10000 ; i++ )
    {
      switch( random(6) )
      {
        case 0:
          tmp_mixed += ({ i });
          break;
        case 1:
          tmp_mixed += ({ i*1.0 });
          break;
        case 2:
          tmp_mixed += ({ i+"" });
          break;
        case 3:
          tmp_mixed += ({ previous_object() });
          break;
        case 4:
          tmp_mixed += ({ i });
          break;
        case 5:
          tmp_mixed += ({ ([ i : i ]) });
          break;
      }
    }
  }
  else
  {
    garray_mixed = ({});
    for( int i = 0 ; i < 10000 ; i++ )
    {
      switch( random(6) )
      {
        case 0:
          garray_mixed += ({ i });
          break;
        case 1:
          garray_mixed += ({ i*1.0 });
          break;
        case 2:
          garray_mixed += ({ i+"" });
          break;
        case 3:
          garray_mixed += ({ previous_object() });
          break;
        case 4:
          garray_mixed += ({ i });
          break;
        case 5:
          garray_mixed += ({ ([ i : i ]) });
          break;
      }
    }
  }
}
