# M1 DESIGN.md

Replace this template with your own concise engineering explanation.

## 1. System structure
The TextProcessor normalizes the text and breaks it into tokens. Next, the Chunker splits the tokens into smaller chunks. The CorpusIndex stores the terms and their different frequencies. The RetrievalEngine searches and ranks chunks based on the query. The ContextBuilder creates the final context while staying within the token budget. The ProcessingCore connects all of these parts together.

## 2. Design decisions
I used vectors to store tokens, chunks, and search results because their order is important. I also used unordered maps to store term frequencies and scores because they make it easy to look up information using a term or chunk. I also used unordered sets to remove repeated query terms and check for duplicate document IDs. Each class handles a separate part of the processing system to keep the code organized.

## 3. Correctness and consistency
I made sure documents and queries use the same normalization rules so terms can be compared correctly. Chunks stay within the maximum token size and include the required overlap. Duplicate document IDs are checked before replacing the current corpus. Search scores are rounded to 12 decimal places, and ties are handled using document and chunk order so the results stay consistent.

## 4. Testing strategy
I built the project with STRICT enabled and ran the provided public tests using CTest. All of the public tests passed. The main things that needed to be tested are normalization, chunking, overlap, paragraph boundaries, term frequencies, search ranking, context token limits, empty inputs, duplicate document IDs, and invalid values of k.

## 5. Alternatives considered
One alternative was to search through every chunk each time a query was made. I didn't choose this because using the CorpusIndex makes it easier to find the chunks that contain each term. Another alternative was to put all of the processing code inside ProcessingCore. I kept the different classes separate instead because it makes the code easier to understand, organize, and test.