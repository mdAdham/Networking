#include "tcpudptesting.hpp"
#include "MultipleSocketWithSocketSelector.hpp"
#include "Simple2DMultiplayerGame.hpp"
#include "SwagSpaceMultiplayerThing.hpp"

using namespace sf;
using namespace std;

int main()
{
	int retval = SwagSpaceMultiplayerThing();

	cin.get();
	return retval;
}