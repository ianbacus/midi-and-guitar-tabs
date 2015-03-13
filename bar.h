
class Bar
//linked list implementation might make most sense here
{
	public:
		vector<Chunk*> Bar_Chunks; //Chunks are children of Bar	

		void add_Chunk(Chunk*);
		//operator [] for accessing Chunks
		
		
		void reconfigure(Base*);
};
