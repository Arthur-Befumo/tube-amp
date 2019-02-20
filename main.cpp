#include <iostream>
#include <csound/csound.hpp>

int main(int argc, char** argv)
{
	Csound* cs = new Csound();
	cs->SetInput("beep.wav");
	return 0;
}