int indexing(string str){
  // Count "e" by using a loop looking at character index in string
  int count = 0;
  int sz = strlen(str);
  for(int i=0;i<sz;i++){
    if(str[i]=='e') count++;
  }
  return count;
}

int exploding(string str){
  // Count "e" using a loop looking at index in an array of strings
  int count = 0;
  int sz;
  string *list;
  list = explode(str,"");
  sz = sizeof(list);
  for(int i=0;i<sz;i++){
    if(list[i][0]=='e') count++;
  }
  return count;
}

string count_chart(){
  int w=100, h=25;
  // w,h are characters, each with 4h 2w dots
  // fill below or a dot per column
  int *grid=allocate(8*w*h);
  int pre,mid,post,a,b,scale;
  string input;
  string output="Each column is 5 chars longer string processed via 2 algorithms, top line means instant\n";
  for(int i=0;i<(2*w);i++){ // going across
    input = sprintf("%*'blargle 's",i*5,"");
    pre = perf_counter_ns();
    indexing(input);
    mid = perf_counter_ns();
    exploding(input);
    post = perf_counter_ns();
    scale = 2000000;
    a = min(({w-1, ((mid-pre)/scale) }));
    //scale = 1000000;
    b = min(({w-1, ((post-mid)/scale) }));
    grid[i + min(({w, (a)}))*2*w] = 1;
    grid[i + min(({w, (b)}))*2*w] = 1;
  }
  for(int i=0;i<h;i++){ // going down
    for(int j=0;j<w;j++){ // going across
      output += sprintf("%c", 0x2800 +
          (grid[i*w*2*4 + j*2] ? 1 : 0) +
          (grid[i*w*2*4 + j*2 + 1] ? 8 : 0) +
          (grid[i*w*2*4 + j*2 + w*2] ? 2 : 0) +
          (grid[i*w*2*4 + j*2 + w*2 + 1] ? 16 : 0) +
          (grid[i*w*2*4 + j*2 + w*4] ? 4 : 0) +
          (grid[i*w*2*4 + j*2 + w*4 +1] ? 32 : 0) +
          (grid[i*w*2*4 + j*2 + w*6] ? 64 : 0) +
          (grid[i*w*2*4 + j*2 + w*6 +1] ? 128 : 0) +
          0);
    }
    output += "\n";
  }
  return output;
}

string count_x(int x){
  string input;
  int pre,mid,post;

  input = sprintf("%*'blargle 's",x,"");
  pre = perf_counter_ns();
  indexing(input);
  mid = perf_counter_ns();
  exploding(input);
  post = perf_counter_ns();
  return sprintf("Input of %d chars, indexing=%1.3f ms, exploding=%1.3f ms\n", strlen(input), (mid-pre)*0.000001, (post-mid)*0.000001);
}


int main(string arg){
  write(count_chart());
  write(count_x(100));
  write(count_x(500));
  write(count_x(1000));
  write(count_x(2000));
  write(count_x(5000));
  write(count_x(10000));
  return 1;
}
