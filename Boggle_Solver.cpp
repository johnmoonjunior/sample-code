//*******************************************************************************************************
//Author: Jack Moon
//Program Name: Boggle Board Solver
//Program Description: Given a dictionary list of legal playable words, a game board's width and height,
//and a list of letters equal in size to the board's width times height, returns a list of all words
//from the given dictionary that can be "solved for" using standard boggle rules as described at
//https://en.wikipedia.org/wiki/Boggle
//Last Updated: 04/13/21
//*******************************************************************************************************
#include <stdio.h>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>

class c_boggle
{
private:
    std::unordered_map<int, std::vector<std::string>> alphabeticalWordContainers; //Hash map to contain sorted lists of legal words, grouped and accessed by first letter
    std::vector<std::string> resultWordsList; //The list of words that will be built and returned by solve_board
    std::vector<bool> isLetterUsed; //Vector array that will be the size of the boggle board, which keeps track of which letters have already been used in the current word
    int checkNextLetter(int currentLetterPosition, int startingLetterValue, std::string &workingWord); //Main function to build word paths
    int searchForSubstring(int currentLetterPosition, int startingLetterValue, std::string &workingWord); //Searches for a matching substring in the corresponding word container
    void insertResultWord(std::string resultWord); //Inserts a found solution word in the vector of result words
    int working_board_width, working_board_height, board_size; //Dimensions of the board being solved
    const char *working_board_letters; //Array of characters that make up the current board

public:
	// prior to solving any board, configure the legal words
	void set_legal_words(
		const std::vector<std::string> &all_words); // alphabetically-sorted array of legal words

	// find all words on the specified board, returning a list of them
	std::vector<std::string> solve_board(
		int board_width,		// width of the board, e.g. 4 for a retail Boggle game
		int board_height,		// height of the board, e.g. 4 for a retail Boggle game
		const char *board_letters);	// board_width*board_height characters in row major order
};

//Sort the list of legal words into containers by first letter; NOTE: this assumes the words come in all lowercase already,
//though it wouldn't be hard to convert them if neccessary
void c_boggle::set_legal_words(const std::vector<std::string> &all_words)
{
    int firstCharValue;
    int previousFirstCharValue = -117;//Keep track of the previous first character to quickly find if we need a new container
    for(std::string word : all_words)
    {
        firstCharValue = word[0];
        if(firstCharValue != previousFirstCharValue)//If it is a new first letter, start a new container in the map
        {
            alphabeticalWordContainers[firstCharValue] =  {word};
        }
        else
        {
            alphabeticalWordContainers[firstCharValue].push_back(word);//Add the word to the end of it's container
        }
        previousFirstCharValue = firstCharValue;
    }
}

//Insert a result word into the solution list in alphabetical position using binary insertion
//This causes the vector's storage capacity to be reallocated with each insert; if we had data on
//the average size this solution list will end up being, we could reserve more space up front to be
//more memory efficient, and add a constant amount each time the vector filled up to capacity.
void c_boggle::insertResultWord(std::string resultWord)
{
    int currentSearchIndex = resultWordsList.size() / 2; //Start index for binary search
    int lowerIndex = 0; //Lower index for binary search
    int upperIndex = resultWordsList.size(); //Upper index for binary search
    int comparisonResult; //Result of comparing 2 strings using str.compare
    bool continueSearch = true;

    while(continueSearch) //Continue looping until the word is inserted
    {
        if(resultWordsList.size() == 0) //If this is the first word added to the list, no search needs to be done.
        {
            resultWordsList.insert(resultWordsList.begin(), resultWord);
            return;
        }

        else if(resultWord > resultWordsList[currentSearchIndex]) //Word is alphabetically later than the one at the current index
        {
            if(currentSearchIndex == resultWordsList.size() - 1) //Word being inserted is alphabetically last so far
            {
                resultWordsList.push_back(resultWord);
                return;
            }
            else if(resultWord < resultWordsList[currentSearchIndex + 1]) //Found the two words it comes alphabetically between, insert here
            {
                resultWordsList.insert(resultWordsList.begin() + currentSearchIndex + 1, resultWord);
                return;
            }
            else
            {
                lowerIndex = currentSearchIndex; //Can't be inserted lower than this
                currentSearchIndex = currentSearchIndex + (((float)upperIndex - currentSearchIndex) / 2); //Get next index location
            }
        }

        else //Word must be alphabetically earlier if it is not later since duplicate words are not allowed
        {
            if(currentSearchIndex == 0) //Word being inserted is alphabetically first so far
            {
                resultWordsList.insert(resultWordsList.begin(), resultWord);
                return;
            }
            else if(resultWord > resultWordsList[currentSearchIndex - 1]) //Found the two words it comes alphabetically between, insert here
            {
                resultWordsList.insert(resultWordsList.begin() + currentSearchIndex, resultWord);
                return;
            }
            else
            {
                upperIndex = currentSearchIndex; //Can't be inserted higher than this
                currentSearchIndex = currentSearchIndex - (((float)upperIndex - lowerIndex) / 2);
            }
        }
    }
}

//Helper function to search the word containers for a matching substring to see if we should continue building the current word
//Uses a binary search to look for words that start with the substring; if a match is found, check if it is a full word
//from the dictionary; if so, add the word to the solution list. Then, run check next letter to see if there are matches with more letters
//Returns: 1 if no problems; -1 if word container is emptied
int c_boggle::searchForSubstring(int currentLetterPosition, int startingLetterValue, std::string &workingWord)
{
    bool continueSearch = true;
    std::vector<std::string> *wordContainer = &alphabeticalWordContainers[startingLetterValue];
    int currentSearchIndex = (*wordContainer).size() / 2; //Get the middle index from the matching word container
    int upperIndex = (*wordContainer).size() - 1, lowerIndex = 0; //The current highest and lowest indexes
    int comparisonResult; //Result of comparing 2 strings using str.compare
    int previousIndex = 0; //Keep track of previous index to see when we run out of words to compare
    int tempIndex = 0; //Temporary index for working through container looking for an exact match to the substring

    while (continueSearch)
    {
        comparisonResult = workingWord.compare((*wordContainer)[currentSearchIndex].substr(0,workingWord.length()));
        if(comparisonResult == 0 || (upperIndex-lowerIndex) < 2) //Substring and a word in the container match, or container is down to 2 words; check for exact match then continue word path
        {
            (comparisonResult == 0) ? tempIndex = currentSearchIndex : tempIndex = upperIndex; //Set the tempIndex based on which case got us here
            //Work back in word container to see if there is an exact match for the substring in the dictionary
            while(workingWord < (*wordContainer)[tempIndex] && tempIndex > 0)
            {
                tempIndex--;
            }
            if(workingWord == (*wordContainer)[tempIndex] && workingWord.length() >= 3) //Indicates an exact match at this index long enough to be a solution word
            {
                insertResultWord(workingWord); //Add word to solution list

                //NOTE: if needed, these words could instead be moved to another location and restored later
                (*wordContainer).erase((*wordContainer).begin() + tempIndex); //Remove word from legal words to reduce search time and avoid duplicates;
            
                if((*wordContainer).empty()) //If that was the last word in the container, remove it from the map
                {
                    alphabeticalWordContainers.erase(startingLetterValue);
                    return -1; //No need to search for more words since the container for this starting letter is now empty
                }
            }

            if(checkNextLetter(currentLetterPosition, startingLetterValue, workingWord) == -1){return -1;}
            else{return 1;} //Break out of function because all paths from this substring have now been checked
        }
        else if(comparisonResult > 0) //Substring is alphabetically after the word at current index
        {
            if(currentSearchIndex == lowerIndex){return 1;} //End of upper bounds; no match
            if (currentSearchIndex == upperIndex - 1) //If we are one index away from the end, we need to just add one to the search index
            {
                previousIndex = currentSearchIndex;
                currentSearchIndex++;
            }
            else //Otherwise continue binary search
            {
                previousIndex = currentSearchIndex;
                lowerIndex = currentSearchIndex;
                currentSearchIndex = currentSearchIndex + (((float)upperIndex - currentSearchIndex) / 2);
            }
        }
               
        else if(comparisonResult < 0) //Substring is alphabetically before the word at current index
        {
            if(currentSearchIndex == upperIndex){return 1;} //End of lower bounds; no match

            if (currentSearchIndex == lowerIndex + 1) //If we are one index away from the lower, we need to just subtract one to the search index
            {
                previousIndex = currentSearchIndex;
                currentSearchIndex--;
            }
            else //Otherwise continue binary search
            {
                previousIndex = currentSearchIndex;
                upperIndex = currentSearchIndex;
                currentSearchIndex -= (((float)currentSearchIndex - lowerIndex) / 2);
            }
        }
        if(previousIndex == currentSearchIndex) {return 1;} //If we have stalled at one index there is no match
    }
    return 1;
}

//Helper function to iterate through each adjacent letter and check if the substring made by adding it to the current
//working work exists in the dictionary of legal words.
//Returns: 1 if no problems; -1 if word container for this starting letter was emptied
int c_boggle::checkNextLetter(int currentLetterPosition, int startingLetterValue, std::string &workingWord)
{
    int nextLetterPosition; //Position of the possible next letter in the word

    //Lambda function perform operations using the current next letter position
    auto letterCheck = [&] () 
    {
        if(!isLetterUsed[nextLetterPosition]) //Make sure the letter hasn't already been used
        {
            workingWord.push_back(working_board_letters[nextLetterPosition]); //Add the next letter to the working word path
            isLetterUsed[nextLetterPosition] = true; //Mark the new letter as used in the current path
            if(searchForSubstring(nextLetterPosition, startingLetterValue, workingWord) == -1)
            {
                return -1;
            }
            workingWord.pop_back(); //Remove the letter from the working word path to check other paths
            isLetterUsed[nextLetterPosition] = false; //Unmark this letter as used for the current path
        }
        return 1;
    };

    //Check the 8 letters surrounding the current one; for each possible position, check first if the current letter has a neighbor at said position
    if(currentLetterPosition >= working_board_width)//Not on top row of board
    {
        if((currentLetterPosition % working_board_width) != 0) //Not on left side of board
        {
            nextLetterPosition = currentLetterPosition - working_board_width - 1; //Top left letter
            if(letterCheck() == -1){return -1;}
        }

            nextLetterPosition = currentLetterPosition - working_board_width; //Top middle
            if(letterCheck() == -1){return -1;}

        if(((currentLetterPosition+1) % working_board_width) !=0) //Not on right side of board
        {
            nextLetterPosition = currentLetterPosition - working_board_width + 1; //Top right
            if(letterCheck() == -1){return -1;}
        }
    }

    if((currentLetterPosition % working_board_width) != 0) //Not on left side of board
    {
        nextLetterPosition = currentLetterPosition - 1; //Middle left
        if(letterCheck() == -1){return -1;}
    }

    if(((currentLetterPosition+1) % working_board_width) !=0) //Not on right side of board
    {
        nextLetterPosition = currentLetterPosition + 1; //Middle right
        if(letterCheck() == -1){return -1;}
    }

    if(currentLetterPosition < (board_size - working_board_width)) //Not on bottom of board
    {
        if((currentLetterPosition % working_board_width) != 0) //Not on left side of board
        {
            nextLetterPosition = currentLetterPosition + working_board_width - 1; //Bottom left
            if(letterCheck() == -1){return -1;}
        }

            nextLetterPosition = currentLetterPosition + working_board_width; //Bottom middle
            if(letterCheck() == -1){return -1;}

        if(((currentLetterPosition+1) % working_board_width) !=0) //Not on right side of board
        {
            nextLetterPosition = currentLetterPosition + working_board_width + 1; //Bottom right
            if(letterCheck() == -1){return -1;}
        }
    }

    return 1;
}

std::vector<std::string> c_boggle::solve_board(int board_width, int board_height, const char *board_letters)
{
    std::string currentWorkingWord = ""; //String that will be built as we go through letter paths
    int startingLetterValue = -117; //ASCII value of the letter at the front of the working word
    int currentLetterPosition = -117; //Position of starting letter on the board
    working_board_letters = board_letters;
    working_board_height = board_height;
    working_board_width = board_width;
    board_size = board_width * board_height;

    if(std::strlen(board_letters) != board_size)
    {
        std::cout << "Number of letters does not match board size!" << std::endl;
        return resultWordsList;
    }

    //Iterate over ever letter on the board as a starting letter
    for(int currentRow = 0; currentRow < board_height; currentRow++){
        for(int currentColumn = 0; currentColumn < board_width; currentColumn++)
        {
            isLetterUsed.assign(board_width*board_height, false); //Reset the used letter list
            currentLetterPosition = currentRow * board_height + currentColumn;
            isLetterUsed[currentLetterPosition] = true; //Mark this letter as used to prevent reuse in current path
            currentWorkingWord = board_letters[currentLetterPosition]; //Start the current working word with the letter at this board position
            startingLetterValue = int(board_letters[currentRow * board_height + currentColumn]); //Get ASCII value of current starting letter
            if(alphabeticalWordContainers.find(startingLetterValue) == alphabeticalWordContainers.end()) //If there is no container of words that start with this letter, move on to the next letter
            {
                continue;
            }
            checkNextLetter(currentLetterPosition, startingLetterValue, currentWorkingWord);
            isLetterUsed[currentLetterPosition] = false; //Unmark this letter as in use
        }
    }
    return resultWordsList;
}


void example_driver()
{
	c_boggle my_boggle;
	std::vector<std::string> my_results;

	my_boggle.set_legal_words({"abed","abo","aby","aero","aery","bad","bade","be","bead","bed","boa","board","bore","bored","box","boy","bread","bred","bro","broad","byre","byroad","dab","deb","derby","dev","dove","oba","obe","orb","orbed","orby","ore","oread","read","reb","red","rev","road","rob","robe","robed","robbed","robber","robed","verb","very","yob","yore"});
	my_results= my_boggle.solve_board(3, 3, "yoxrbaved");
    for (std::string word : my_results)
    {
        std::cout << word << std::endl;
    }
}

int main()
{
    example_driver();
}

