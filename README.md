# RedPantsDeluxe2
The long awaited sequel

# Boot
`python boot.py`
run with -e for the editor
run with -R to reload scripts
run with -W % or -H % to set custom screen size

### The Red Pants Engine
(this is all very much subject to change)
There are Worlds with Actors
There are Actors with Scripts.
Scripts are in a custom language I'm lovingly calling RedPantsScript.

# Using the engine
(very much subject to change)
currently, the editor is not so far along.
Scripts should be in /scripts and end with `.rp`
Spritesheets should be in /img and end with `.png`

Boot the editor with `python boot -e`
 - Load scripts with `Shift+L`
 - Edit spritesheet with `Shift+S`
 - Save with `CTRL+S`
 - Exit with `ESCAPE`
 - use `CTRL+ESCAPE` to force exit without saving

### Red Pants Script
Structure:
at the top of your script you have to define some things
`{name}|{x},{y},{w},{h}|{tangible?}|{x sprite offset},{y sprite offset}|`
I know thats annoying, as I build the editor this will go away and become menu items or something.

Immediately after, you define your spritesheet. Names of sprites are defined in the editor spritesheet menu.
```
{state}:{frame} {spritename}
{state}:{frame} {spritename}
...
```
```
IDLE:0 happyguystand0
IDLE:5 happyguystand1
```
Everything else in the program will be code blocks. 
  In the same way we defined our spritesheet, the game will refrence scripts as `{state}:{frame}`
  So if your actor's state is IDLE and hes on frame 0, any codeblock called IDLE:0 will be run.
  If there is NO codeblock at the specific index, it will scan backwards until it either reaches zero or finds an index
  That means, if you want a code block to run for frames 4 through 6 on state IDLE, you would define it as 
  IDLE:4
  and then define another code block as
  IDLE:7
  which would do something else, or nothing.
  
Structure for these statements are as like this
```
|IDLE:0|
{code...}
{code...}

|IDLE:4|
{code...}
{code...}

|someOtherCodeBlock|
{code...}
{code...}

```

Statements in Red Pants Script have three steps to resolve
 - Evaluate
    - in this step, we handle refrences.
      - objects should be refrenced with their NAME
      - understands the python concept of self
      - dot notation `self.x` `platform.SomeVariable`
    - we also can refrence the input state
      - uses syntax `inp{name}` where name is the key to the input field
      - edit the inputs.py file to change how you want inputs to be rendered, though make sure to leave the `EVENTS` key
      - `inpLEFT` `LEFT_DOWN in inpEVENTS
 - Operators
    - in this step, we handle operators strictly from left to right
    - valid operators are
         - +, -, *, //, /, %, **,
         - ==, >=, <=, >, <, !=,
         - and, or, nor, not, in
         - abs (absolute value)
 - Commands
    - in this step we handle commands.
    - each statement has to follow the syntax for any specific command.
    - commands and syntax are:
        - set - Set an Actor attribute
             - `set {actor key (or self)} {variable name} {value to be set}`
             - `set self x_velocity self.x_velocity + 1`
        - if  - I hope you've heard of this one
             - ```
               if {conditional}
               ...
               endif
               ```
             - ```
               if self.name == PLAYER
               ...
               endif
               ```
        - exec - runs another code block
            - `exec {key}`
            - `exec applyGrav`
        - img - overrules the sprite for the frame with a new image key
            - `img {new image key}`
            - `img happyguysad`
        - print - prints something, good for debugging
            - `print {whatever}`
            - `print hello-world`
        
Notes:
. Any other token will be attempted to evaluate as an int or float in the Evaluate step, otherwize they are all one word strings
. All of this is done behind the scenes with string.split() and string.splitlines() so you have to have whitespace seperating every token

Good Luck! :)
-WXLY
