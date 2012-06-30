-- Notes
Here is LaTeX code for my thesis, structured in the following manner:
- main.tex: min file to compile
- _meta.tex: abbreviations an user defined commands
- _packages.tex: the used packages with the options described 
- _pagestyle.tex: headings and page geometry
- _hyphenation.tex: manual syllabification, especially for 'overfull' cases
- _empty.tex: used for inserting a blank page after cover (hack since tex is buggy)
- content/<…>: the text separated into chapters
- pics/<…>: the .jpg and .png that were used in the text
- citation.bib: the Bibtex file with the reference information
- scripts/: scripts that I wrote to facilitate the use of TeXShop (Mac)

-- Environment
- using UTF-8(!), can cause errors but way more convenient
- TeXShop 2.43, I'm not sure if any additional packages were installed, should use the out-of-the-box configuration though 