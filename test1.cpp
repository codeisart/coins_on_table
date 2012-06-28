#include <iostream>

#define TRACE_ONx

#ifdef TRACE_ON
#define DEBUG_OUT(m) std::cerr << m
#else 
#define DEBUG_OUT(m) 
#endif

class Board
{
public:
    Board(int xMax, int yMax) 
        : m_board(0),
          m_time(0),
          m_maxX(xMax),
          m_maxY(yMax),
          m_coinX(0),
          m_coinY(0),
          m_endX(0),
          m_endY(0)
    {
        m_board = new char[xMax*yMax];
    }
    ~Board()
    {
        delete[] m_board;
    }

   	void resetBall()
	{
		m_coinX = 0;
		m_coinY = 0;	
	}
    
    int linearPos(int x, int y) { return x+(m_maxX * y); }
    
   	bool isOutOfBounds() const
	{		
		return m_coinX < 0 || m_coinY < 0 || m_coinX >= m_maxX || m_coinY >= m_maxY;		
	}
  
    void addRow(const char* row, int yIndex)
    {
    	int index = linearPos(0,yIndex);
		for(;*row; row++,index++)
		{
			m_board[index] = *row;
    		
			// remember where the end is.
			if(*row == '*')
			{
				m_endX = index % m_maxX;
				m_endY = yIndex;	
			}	
		}
	}
	
	int intabs(int v) { return v < 0 ? -v : v; }
    
    int generate(int neededLength, int currentLength)
	{
		using namespace std;

		int delta = neededLength - currentLength;
		int changeCount = 0;

		// Need to make shorter.
		if(delta < 0)
		{
			int deltaX = m_endX;
			int deltaY = m_endY;
			int cy = 0;
			int cx = 0;

			// Impossible?
			if(deltaX+deltaY > neededLength)
			{
				DEBUG_OUT( "Impossible" << endl);
				return -1;
			}

			DEBUG_OUT("End X: " << m_endX << endl);
			DEBUG_OUT("End Y: " << m_endY << endl);
			DEBUG_OUT("Making shorter.." << delta << endl);
			DEBUG_OUT("DeltaX" << deltaX << endl);	
			DEBUG_OUT("DeltaY" << deltaY << endl);	
						
			for(int i=0;i<deltaX; i++,cx++)
			{	
				int index = linearPos(cx,cy);
				char& c = m_board[index];
				if(c!= 'R')
				{
					c = 'R';
					changeCount++;
					DEBUG_OUT("g");
				}	
			}
			
			for(int i=0;i<deltaY; i++,cy++)
			{	
				int index = linearPos(cx,cy);
				char& c = m_board[index];
				if(c!= 'D')
				{
					c = 'D';
					changeCount++;
					DEBUG_OUT("g");
				}	
			}
			DEBUG_OUT(endl);
		}
		
		return changeCount;
	}

	bool tick()
    {
        bool resting = false; 
		char c = m_board[linearPos(m_coinX,m_coinY)]; 
        switch(c)
        {           
			case 'U': 
                m_coinY--; 
                break;
            case 'D': 
                m_coinY++;
                break;
            case 'L':
				m_coinX--;
				break;
            case 'R':
				m_coinX++;
				break;
            case '*':
				resting = true;
				break;
			default:
				DEBUG_OUT("Bad data in tick " << c << std::endl);
		}
		DEBUG_OUT("t");
		return resting;
    }

	void display()
	{
		using namespace std;
		DEBUG_OUT( endl );
		for(int y = 0; y < m_maxY; ++y)
		{	
			for(int x = 0; x < m_maxX; x++)
				DEBUG_OUT( m_board[linearPos(x,y)]);
			DEBUG_OUT( endl );
		}	
	}
    
private:
        char* m_board;
        int m_time;
        int m_maxX;
        int m_maxY;
        int m_coinX;
        int m_coinY;
        int m_endX;
        int m_endY;
};
    
int main(int argc, char** argv)
{
	using namespace std;
	
	try
	{
		int rows, cols, nIters;
		// Assume input data is sane.
		cin >> rows;
		cin >> cols;
		cin >> nIters;

		DEBUG_OUT( "Processing " << rows << " rows, " << cols << " cols, " << nIters << " iters." << endl);

		// Sane data?
		if(!(rows > 0 && cols > 0 && nIters>=0))
		{
			DEBUG_OUT( "Data not sane " << endl);
			cout << -1;
			return 0;
		}
		Board b(cols,rows); 

		string txt;
		getline(cin,txt);	// read the eol from above before starting loop.
		for(int i= 0; i< rows; ++i)
		{
			getline(cin,txt);
			b.addRow(txt.c_str(),i);
			/*if(	txt.length()-1 != cols)
			{
				DEBUG_OUT("Not enough data in a row... read " << txt.length()-1 << ", and expected " << cols << endl);
				DEBUG_OUT(txt);
				cout << -1;
				return 0;
			}
			*/
		}

		b.display();

		// How many till rest or out of bounds.
		// Protect against infite loops.
		int tickCount = 0;
		const int maxTicks = (cols*rows)+1;
		while( !b.tick() && !b.isOutOfBounds() && tickCount < maxTicks) 
			++tickCount;

		DEBUG_OUT(endl);
		if(tickCount >= maxTicks)
		{
			DEBUG_OUT("Coin never came to rest. Tick Count=" << tickCount << ", MaxTicks = " << maxTicks << endl);
			tickCount = maxTicks;
		}

		// Bad code?
		else if(b.isOutOfBounds())
		{
			DEBUG_OUT("Out of bounds, but attempting to process anyway." << endl);
			tickCount = maxTicks;
		}
		else
		{
			DEBUG_OUT("Board takes " << tickCount << " to complete" << endl);
		}	
	
		int generated = b.generate(nIters,tickCount);
		cout << generated;
			
		b.display();
	}
	catch(...)
	{
		// Something bad happened. Most likely bad formated data.
		cout << -1;
	}
	//cout << endl; // new line at eof
}
