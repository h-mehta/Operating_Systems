#include<iostream>
#include<string.h>
#include<fstream>
#include<stdlib.h>
#include<stdio.h>
#include<cstring>
#include<ctype.h>
#include<vector>
#include<typeinfo>
#include<map>
#include<set>

using namespace std;

int token_ptr = 0;		// points to current token
int no_of_tokens = 0;		// no of tokens processed
int module_offset = 0;		// to create module offset table

vector<string> tokens;
vector<int> line_number;
vector<int> line_offset;

map<string,int> symbolTable;		// GEST table
map<string,string> symbolTableError;	

vector<int> global_module_offset;

string getToken(){
	string temp_token = tokens[token_ptr];
	token_ptr++;
	return temp_token;
}

void populate_structures(char** file){
	
	//Opening the file
	ifstream file1(file[1]);
	
	if(file1.is_open()){
        string line;
        int lno = 0;
        int loffset = 1;
		int eof_newline_flag = 0;
		int final_line;
		int final_offset; 
        while(getline(file1, line)){
                lno++;
                loffset = 1;
                if(line[0]=='\0'){
                    //cout << "\n entered here ";
                    eof_newline_flag++;
                    final_line = lno-1;
                    final_offset = loffset;
                    continue;
                }else{
                            eof_newline_flag = 0;
                        	char *token;
				
                        	int n = line.length();
                        	char char_array[n+1];
                        	strcpy(char_array, line.c_str());

                            for(int i=0; i<n; i++){
                                	if(char_array[i]==' ' or char_array[i]=='\t'){
                                        	char_array[i] = '$';
                                    }
                                }

                        	token = strtok(char_array,"$");
				
                        	while(token!=NULL){
                                	no_of_tokens++;
                                	tokens.push_back( token );
                                    line_number.push_back( lno  );
                                	line_offset.push_back( loffset );
					
                                    int token_len = 0;
                                    char ch = token[0];
                                    while(ch!='\0'){
                                        token_len++;
                                        ch = token[token_len];
                                    }
                                    token_len--;
                                    
                                    loffset = loffset + token_len + 2;
                                    token = strtok(NULL,"$");
                        	}
                	}
        }

		tokens.push_back("\0");
		if(eof_newline_flag>=2){
			//cout << "\n entered wrong block ";
			line_number.push_back( final_line );
			line_offset.push_back( final_offset );		
		}else{
			line_number.push_back( line_number[no_of_tokens-1]  );
			line_offset.push_back( line_offset[no_of_tokens-1] + tokens[no_of_tokens-1].length() );
		}
        
	}else{
                cout << "\n Couldn't Open file " << endl;
        }		
}

int readInt(){
	string a = getToken();

	string num = a;
        int length_of_str = num.length();
        char num_array[length_of_str+1];
        strcpy(num_array, num.c_str());
        int no_of_chars = 0;
        while(isdigit(num_array[no_of_chars])){
                no_of_chars++;
        }
        if(no_of_chars!=length_of_str){
                return -1;
        }


	int val = 0;
	int len = a.length();
	for(int i=0; i<len; i++){
		val = val*10;
		val = val + int(a[i]) - 48;
	}
	return val;
}

string readSymbol(){
	string symb = getToken();

	int len_of_symbol = symb.length();
        char alpha1_array[len_of_symbol+1];
        strcpy(alpha1_array, symb.c_str());
        int len_counter = 0;
	if(!isalpha(alpha1_array[0])){
		return "?";
	}
        while(isalnum(alpha1_array[len_counter])){
                len_counter++;
        }
        if(len_of_symbol!=len_counter){
                return "?";	// ? - signifies error
        }

	return symb;
}

char readIEAR(){
	string a = getToken();
	char addressmode = a[0];
	if(addressmode == 'I' or addressmode == 'E' or addressmode == 'A' or addressmode == 'R'){
		return addressmode;
	}else{
		return '?';	// signifies error
	}
}

void update_global_module_offset(int moff){
	global_module_offset.push_back(moff);
}

void update_module_offset(int instcount){
	update_global_module_offset(module_offset);
	module_offset += instcount;
}

// Creating GEST table
bool createSymbol(string symb, int val){
	//checking if symb already exists in the Symbol table
	map<string,int>::iterator sstit;
	sstit = symbolTable.find(symb); 
	if(sstit!=symbolTable.end()){
		// symbol already exists in the symbolTable
		return false;
	}else{
		symbolTable[symb] = val;
		return true;
	}
}

//Search the symbol table
int searchSymbolTable(string symb){
	map<string,int>::iterator it;
	it = symbolTable.find(symb);
	if(it!=symbolTable.end()){
		// element found in symbol table
		return it->second;
	}else{
		//element not found in symbol Table
		return -1;
	}
}

map<string,int>::iterator symbolTable_it;
string last_variable_searched;
void check_symbolTable(int instcount, int module_number){
	if(symbolTable.size()==0)
		return;
        if(module_number==0){
                symbolTable_it = symbolTable.begin();
        }else{
                symbolTable_it = symbolTable.find(last_variable_searched);
                symbolTable_it++;
        }
        while(symbolTable_it!=symbolTable.end()){
                int module_offset = global_module_offset[ module_number];
                if(symbolTable_it->second - module_offset >= instcount ){
                        cout << "Warning: Module "<<module_number+1<<": "<<symbolTable_it->first<<" too big "<<symbolTable_it->second-module_offset<<" (max="<< instcount-1 <<") assume zero relative"<<endl;
                        symbolTable_it->second = module_offset;
                }
                last_variable_searched = symbolTable_it->first;
                symbolTable_it++;
        }
}

void parse_error_message(int errno, int error_line_no, int error_line_offset){
	if(errno==1){
		cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": NUM_EXPECTED"<<endl;
	}
	else if(errno==2){
                cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": SYM_EXPECTED"<<endl;
        }
	else if(errno==3){
		cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": ADDR_EXPECTED"<<endl;
	}else if(errno==4){
                cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": SYM_TOO_LONG"<<endl;
        }else if(errno==5){
		cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": TOO_MANY_DEF_IN_MODULE"<<endl;
	}else if(errno==6){
		cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": TOO_MANY_USE_IN_MODULE"<<endl;
        }else if(errno==7){
                cout << "Parse Error line "<<error_line_no<<" offset "<<error_line_offset<<": TOO_MANY_INSTR"<<endl;
        }
	else{
		cout << "Wrong error number";
	}
	exit(0);
}

int main(int argc, char** file){
	if(argc!=2){
		cout << " Wrong format " << endl;
		return 1;
	}
	
	populate_structures(file);
	/*
	cout << "\n size of vector : "<<tokens.size()<<endl;
                int counter2 = 0;
                for(int it = 0; it<tokens.size();it++){
                        cout << "\n Token "<<counter2<< " : "<<tokens[it] << " \t Line "<<line_number[it]<<"\n Line offeset" << line_offset[it] << endl;
                        counter2++;
                }
	*/
	//Pass 1
	token_ptr = 0;
	int total_no_instr = 0;		// stores total number of instructions
	set<string> double_declaration;
	int module_number = 0;
	while( token_ptr<no_of_tokens  ){
		
		// Definition List
		int defcount = readInt();
		if(defcount==-1){
                          parse_error_message(1,line_number[token_ptr-1],line_offset[token_ptr-1]);
                }
		if(defcount>16){
                          parse_error_message(5,line_number[token_ptr-1],line_offset[token_ptr-1]);
                }
		for(int i=0; i<defcount; i++){
			string symb = readSymbol();
			if(symb[0]=='?'){
			     parse_error_message(2,line_number[token_ptr-1],line_offset[token_ptr-1]);
			}
			if(symb.length()>16){
			     parse_error_message(4,line_number[token_ptr-1],line_offset[token_ptr-1]);
			}
			int val = readInt();
			if(val==-1){
			  parse_error_message(1,line_number[token_ptr-1],line_offset[token_ptr-1]);
			}
			bool flag = createSymbol(symb,val+module_offset);
			if(!flag){
				// duplicate declaration 
				double_declaration.insert(symb);
			}
		}

		// Use List
		int usecount = readInt();
		if(usecount==-1){
                          parse_error_message(1,line_number[token_ptr-1],line_offset[token_ptr-1]);
                }
		if(usecount>16){
                          parse_error_message(6,line_number[token_ptr-1],line_offset[token_ptr-1]);
                }
		for(int i=0; i<usecount; i++){
			string symb = readSymbol();
			if(symb[0]=='?'){
                             parse_error_message(2,line_number[token_ptr-1],line_offset[token_ptr-1]);
                        }
			if(symb.length()>16){
                             parse_error_message(4,line_number[token_ptr-1],line_offset[token_ptr-1]);
                        }
		}       

		//Program text
		int instcount = readInt();
		if(instcount==-1){
                          parse_error_message(1,line_number[token_ptr-1],line_offset[token_ptr-1]);
                }
		total_no_instr = total_no_instr + instcount;
		if( total_no_instr > 512 ){
			  parse_error_message(7,line_number[token_ptr-1],line_offset[token_ptr-1]);
		}
		for(int i=0; i<instcount; i++){
			char addressmode = readIEAR();
			if(addressmode=='?'){
                             parse_error_message(3,line_number[token_ptr-1],line_offset[token_ptr-1]);
                        }
			int operand = readInt();
			if(operand==-1){
                          parse_error_message(1,line_number[token_ptr-1],line_offset[token_ptr-1]);
                	}
		}
		update_module_offset(instcount);

		check_symbolTable( instcount, module_number );
                module_number++;
	} 

	//Printing Symbol Table
	
	cout <<"Symbol Table"<<endl;
	for(map<string,int>::iterator pstit = symbolTable.begin(); pstit!=symbolTable.end(); ++pstit)
	{
		set<string>::iterator ddit;
                ddit = double_declaration.find(pstit->first);
                if(ddit==double_declaration.end()){
                        cout << pstit->first << "=" << pstit->second << endl;
                }else{
                        cout << pstit->first << "=" << pstit->second<<" Error: This variable is multiple times defined; first value used" << endl;
                }

	} 

	//Pass 2
	token_ptr = 0;			// reset to 0 to go over tokens
	int counter = 0;		// to print variable number in memory map
	int module_number2 = 0;		// keep track of module_number to resolve Relative addr
	map <string, int> declaration_list_symbols; // keep track of symbols in declaration list 
	set <string> use_list_symbols; 	// keep track of symbols in use list
	cout <<"\nMemory map"<<endl;
	while( token_ptr<no_of_tokens  ){
		
		vector<int> lesa_table;	
		vector<string> lesa_variable;
		set<string> actually_used_variable;

		int defcount = readInt();
		for(int i=0; i<defcount; i++){
                        string symb = readSymbol();
                        int val = readInt();
			
			// Storing symbols defined in declaration list
			map <string,int>::iterator dlit = declaration_list_symbols.find(symb);
                        if(dlit==declaration_list_symbols.end()){
                                declaration_list_symbols[symb] = module_number2;
                        }
                }

		int usecount = readInt();
		for(int i=0; i<usecount; i++){
                        string symb = readSymbol();
			int addr = searchSymbolTable(symb);
			lesa_table.push_back(addr);
			lesa_variable.push_back(symb);
		
			use_list_symbols.insert(symb);
                }

		int instcount = readInt();
		for(int i=0; i<instcount; i++){
                        char addressmode = readIEAR();
                        int operand = readInt();
			if(addressmode=='I') {
				if(operand >= 10000){
                                        operand = 9999;
                                        printf("%03d: %04d Error: Illegal immediate value; treated as 9999\n",counter,operand);
                                        counter++;
                                }else{
				printf("%03d: %04d\n",counter,operand);
				counter++;
				}
			}else if(addressmode=='A'){
				if(operand >= 10000){
                                        operand = 9999;
                                        printf("%03d: %04d Error: Illegal absolute value; treated as 9999\n",counter,operand);
                                        counter++;
                                }
				else if(operand%1000>=512){
                                	int modified_operand = operand-operand%1000;
                                	printf("%03d: %04d Error: Absolute address exceeds machine size; zero used\n",counter,modified_operand);
                                        counter++;
                                }else{
                                        printf("%03d: %04d\n",counter,operand);
                                        counter++;
                                }	

			}else if(addressmode=='E'){
				int index = operand%100;
				if(operand >= 10000){
                                        operand = 9999;
                                        printf("%03d: %04d Error: Illegal external value; treated as 9999\n",counter,operand);
                                        counter++;
                                }
				else if(index<16 and index<lesa_table.size()){
					if(lesa_table[index] == -1){
					        operand -= index;
                                        	printf("%03d: %04d ",counter,operand);
                                        	cout << "Error: "<<lesa_variable[index]<<" is not defined; zero used"<<endl;
                                        	counter++;
						actually_used_variable.insert(lesa_variable[index]);
                                	}else{
						operand -= index;
						operand += lesa_table[index];
						printf("%03d: %d\n",counter,operand);
                                		counter++;
						actually_used_variable.insert(lesa_variable[index]);
					}
				}else{
					printf("%03d: %04d Error: External address exceeds length of uselist; treated as immediate\n",counter,operand);
					counter++;
				}
			}else if(addressmode=='R'){
				if(operand >= 10000){
					operand = 9999;
					printf("%03d: %04d Error: Illegal opcode; treated as 9999\n",counter,operand);
					counter++;
				}else{	
				int size_of_module = instcount-1;
                                if( operand%1000 > size_of_module ){
                                        operand = operand - operand%1000;
                                        operand += global_module_offset[module_number2];
                                        printf("%03d: %04d Error: Relative address exceeds module size; zero used\n",counter,operand);
                                        counter++;
                                }else{
                                        operand += global_module_offset[module_number2];
                                        printf("%03d: %04d\n",counter,operand);
                                        counter++;
                                }
				}
			}else{
				cout <<"\n Invalid address mode ";
			}
                }
		//printing warning messages if variable in LESA but not used
		for(int i=0; i<lesa_variable.size() ; i++){
			string variable_in_lesa = lesa_variable[i];
			set<string>::iterator auvit = actually_used_variable.find(variable_in_lesa);
			if(auvit == actually_used_variable.end() ){ //and lesa_table[i]!=-1){
				cout << "Warning: Module "<<module_number2+1<<": "<<variable_in_lesa<<" appeared in the uselist but was not actually used" << endl;
			}
		}
		module_number2++;
	}

	// Crossing out use list symbols
	for(set<string>::iterator usit = use_list_symbols.begin(); usit!=use_list_symbols.end(); ++usit)
	{
                map<string,int>::iterator dsit = declaration_list_symbols.find(*usit);
                if(dsit!=declaration_list_symbols.end()){
			declaration_list_symbols.erase(dsit);
		}
        }

	// Printing symbols that were declared but not used
	if(declaration_list_symbols.size()!=0){
                for(map<string,int>::iterator dsit = declaration_list_symbols.begin(); dsit!=declaration_list_symbols.end(); ++dsit){
                        cout<<endl<<"Warning: Module "<<(dsit->second)+1<<": "<<dsit->first<<" was defined but never used";
                }
        }	
	return 0;
}
