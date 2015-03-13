class Note : public Base 
{
	public:
		Note(int);
		int pitch;
		int current_note_index;
		

		int get_fret() const;
		int get_string() const;
		void increment_note_index();
		//int get_current_note_index const {return current_note_index;}
		
		virtual bool compare(Note*) const;
	
//		void set_fret_string();
	

};
