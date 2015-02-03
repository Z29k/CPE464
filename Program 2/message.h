#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>


class Message {
	uint32_t sequence_number;
	uint8_t flag;
	uint8_t to_length;
	char *to;
	uint8_t from_length;
	char *from;
	int text_length;
	char *text;
	uint8_t *bytes;
	
	public:
	
	Message();
	Message(const char *text);
	Message(uint8_t *recieved, int length);
	~Message();
	
	void set_sequence_number(uint32_t number);
	void set_flag(uint8_t flag);
	void set_to(char *to, int length);
	void set_from(char *from, int length);
	void set_text(char *text, int length);
	
	uint32_t get_sequence_number();
	uint8_t get_flag();
	char *get_to();
	char *get_from();
	char *get_text();
	int get_length();
	
	int pack();
	uint8_t *sendable();
	
	private:
	
	void unpack();
};

#endif