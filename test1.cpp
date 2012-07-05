#include <iostream>
#include <vector>
#include <math.h>
#include <functional>
#include <utility>
#include <algorithm>
#include <cctype>

#define TRACE_ON

#ifdef TRACE_ON
	#define DEBUG_OUT(m) std::cerr << m
	#define DEBUG_ASSERT(x,m) if(!(x)) { std::cerr << "Assert Fail: " << m << std::endl; for(;;); }
#else 
	#define DEBUG_OUT(m) 
	#define DEBUG_ASSERT(x,m)
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

   	void reset()
	{
		m_coinX = 0;
		m_coinY = 0;	
	}
    
    int linearPos(int x, int y) { return x+(m_maxX * y); }
    
	bool isOutOfBounds(int x,int y) const
	{
		return x < 0 || y < 0 || x >= m_maxX || y >= m_maxY;		
	}
   	bool isOutOfBounds() const
	{		
		return isOutOfBounds(m_coinX,m_coinY);		
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
    
	struct Node
	{	
		Node(int posx=0,int posy=0,int cfs=0,int ctg=0,Node* p=0) 
			: x(posx),y(posy),costFromStart(cfs),costToGoal(ctg),costTotal(0),parent(p) {}
		int x, y;
		int costFromStart;
		int costToGoal;
		int costTotal;
		Node* parent;
	};

	int sqr(int x) const { return x*x; }
	int distanceSqr(int x1, int y1, int x2, int y2) { return sqr(x1 - x2)+sqr(y1 - y2); }
	int distance(int x1, int y1, int x2, int y2) { return sqrt(distanceSqr(x1,y1,x2,y2)); }	
	int manhattan(int x1, int y1, int x2, int y2) { return intabs(x2-x1) + intabs(y2-y1); }	
	char atBoard(int x,int y) { return m_board[linearPos(x,y)]; }

	struct EqualsNode : public std::binary_function<const Node*,const Node*,bool>
	{
		bool operator()(const Node* lhs, const Node* rhs) const 
		{ return lhs->x == rhs->x && lhs->y == rhs->y; }
	};	
	struct LessNode : public std::binary_function<const Node*,const Node*,bool>
	{
		bool operator()(const Node* l, const Node* r) const
		{ return l->costTotal < r->costTotal; }
	};

	Node* scoreAndValidate(Node* n, char orient)
	{
		if(isOutOfBounds(n->x,n->y)) 
			return 0;
	
		int cost = 1;
		if(n->parent)
		{
			cost += n->parent->costFromStart;
			char c = atBoard(n->parent->x,n->parent->y);
			if(c != orient)
				cost++;			
		}
		
		n->costFromStart = cost;
		n->costToGoal = manhattan(n->x,n->y,m_endX,m_endY);
		n->costTotal = n->costToGoal+n->costFromStart;
						
		//DEBUG_OUT("cost to Goal:" << n->costToGoal << " cost to start" << cost << " total: " << n->costTotal << std::endl);

		return n;	
	}

	char orient(int x, int y, Node* relativeTo)
	{
		//DEBUG_ASSERT(x != y, "Bad Orient");
		//DEBUG_ASSERT(
		//	(x != 0 && y == 0)||
        //   (y != 0 && x == 0),
		//		"Diagonal Orientations not supported: " << x << ", " << y);

		if(x < relativeTo->x) return 'L';
		else if(x > relativeTo->x) return 'R';
		else if(y < relativeTo->y) return 'U';
		else if(y > relativeTo->y) return 'D';

		return '!';
	}

	int aStarSearch(int maxIters)
	{
		using namespace std;
		typedef vector<Node*> NodeContainer; 
		NodeContainer closedList;
		NodeContainer openList;

		// Start with starting position.
		{
			Node* n = new Node();
			n->costToGoal = manhattan(0,0,m_endX,m_endY);
			openList.push_back(n);
		}
	
		bool routeFound = false;
		int diffCount = 0;
		int length = 0;
		while(!openList.empty())
		{
			// Find item in open list with lowest cost
			NodeContainer::iterator nodeIter = min_element(openList.begin(),openList.end(),LessNode());
			Node* node = *nodeIter;
			
			// Remove from open list.
			*nodeIter = openList.back();
			openList.pop_back();
	
			// Add to closed list.
			closedList.push_back(node);			
	
			// Is this the end?	
			if(node->x == m_endX && node->y == m_endY)
			{
				// Construct a path backwards from node to start.
				for(Node* i = node; i; i = i->parent,length++)
				{
					DEBUG_OUT("index " << length << " costToStart: " << i->costFromStart << " costToGoal: " 
							<< i->costToGoal << " costTotal" << i->costTotal <<
								" route x: " << i->x << " route y: " << i->y << endl);				
				
					if( i->parent )
					{						
						char o = orient(i->x,i->y,i->parent);						
						int index = linearPos(i->parent->x,i->parent->y);
						if( m_board[index] != o)
						{
							m_board[index] = o;
							diffCount++;
						}
					}
				}
			
				length--; // Not count the root.	
				routeFound = true;
				break; // success;
			}
			else
			{
				// Add all neighbouring cells
				typedef std::pair<Node,char> NodePair;
				NodePair nrbs[] = 
				{
					NodePair(Node(node->x-1,node->y,0,0,node), 'L'),
					NodePair(Node(node->x+1,node->y,0,0,node), 'R'),
					NodePair(Node(node->x, node->y+1,0,0,node),'D'),
					NodePair(Node(node->x, node->y-1,0,0,node),'U')
				};
				
				for(int i = 0; i < 4; i++)
				{
					if( Node* n = scoreAndValidate(&nrbs[i].first, nrbs[i].second))
					{
						NodeContainer::iterator itClosed = 
							std::find_if(closedList.begin(),closedList.end(),std::bind2nd(EqualsNode(),n));
						NodeContainer::iterator itOpen = 
							std::find_if(openList.begin(),openList.end(),std::bind2nd(EqualsNode(),n));	

						// Already processed this neighbour?
						if(itClosed != closedList.end())
						{
							// If already in list, update the item if this one is cheaper.
							Node* c = *itClosed;
							if( n->costFromStart < c->costFromStart )
								*c = *n;
						}
						// Add to open list if it's not already.
						else if(itOpen != openList.end())
						{
							// If already in list, update the  item if this one is cheaper.
							Node* c = *itOpen;
							if( n->costFromStart < c->costFromStart )
								*c = *n;
						}					
						else
						{
							openList.push_back(new Node(*n));
						}	
					}		
				}
			}
		}
		for( NodeContainer::iterator i = openList.begin(); i!= openList.end(); ++i)
			delete *i;
		
		for( NodeContainer::iterator i = closedList.begin(); i!= closedList.end(); ++i)
			delete *i;

		if(!routeFound)
		{			
			DEBUG_OUT("Fail to find a route?!" << endl);
			return -1;
		}

		if(length-1 > maxIters)
		{
			DEBUG_OUT("Optimal route too long " << length << " limit is " << maxIters << endl);
			return -1;
		}

		DEBUG_OUT("Route found at length " << length << " with " << diffCount << " diffs" << endl);

		return diffCount;
	}

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
			if(	txt.length() != cols)
			{
				DEBUG_OUT("Not enough data in a row... read " << txt.length() << ", and expected " << cols << endl);
				DEBUG_OUT(txt);
				cout << -1;
				return 0;
			}	
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
	
		// Need to do any work?
		int diffCount = 0;
		if(tickCount > nIters)
			 diffCount = b.aStarSearch(nIters);

		cout << diffCount;	
	
		//int generated = b.generate(nIters,tickCount);
		//cout << generated;
		
		if( diffCount >= 0)
		{
			b.reset();
			tickCount = 0;
			while( !b.tick() && !b.isOutOfBounds() && tickCount < maxTicks) 
				++tickCount;
		
			DEBUG_OUT((tickCount <= maxTicks ? "Success" : "Fail!") << endl);
		}
		b.display();
	}
	catch(...)
	{
		// Something bad happened. Most likely bad formated data.
		cout << -1;
	}
	//cout << endl; // new line at eof
}
