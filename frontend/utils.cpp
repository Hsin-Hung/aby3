#include "utils.h"

std::vector<std::pair<std::string, std::vector<int>>>
read_csv(std::string filename)
{
    // Reads a CSV file into a vector of <string, vector<int>> pairs where
    // each pair represents <column name, column values>

    // Create a vector of <string, int vector> pairs to store the result
    std::vector<std::pair<std::string, std::vector<int>>> result;

    // Create an input filestream
    std::cout << filename << std::endl;
    std::ifstream myFile(filename);

    // Make sure the file is open
    if (!myFile.is_open())
        throw std::runtime_error("Could not open file");

    // Helper vars
    std::string line, colname;
    int val;

    // Read the column names
    if (myFile.good())
    {
        // Extract the first line in the file
        std::getline(myFile, line);

        // Create a stringstream from line
        std::stringstream ss(line);

        // Extract each column name
        while (std::getline(ss, colname, ','))
        {

            // Initialize and add <colname, int vector> pairs to result
            result.push_back({colname, std::vector<int>{}});
        }
    }

    // Read data, line by line
    while (std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);

        // Keep track of the current column index
        int colIdx = 0;

        // Extract each integer
        while (ss >> val)
        {

            // Add the current integer to the 'colIdx' column's values vector
            result.at(colIdx).second.push_back(val);

            // If the next token is a comma, ignore it and move on
            if (ss.peek() == ',')
                ss.ignore();

            // Increment the column index
            colIdx++;
        }
    }

    // Close file
    myFile.close();

    return result;
}

Party_IPs getPartyIPs()
{

    Party_IPs party_ips;

    std::fstream newfile;
    newfile.open(IP_FILE_NAME, std::ios::in); // open a file to perform read operation using file object
    if (newfile.is_open())
    { // checking whether the file is open
        getline(newfile, party_ips.ip0);
        getline(newfile, party_ips.ip1);
        getline(newfile, party_ips.ip2);
        newfile.close(); // close the file object.
    }

    return party_ips;
}
