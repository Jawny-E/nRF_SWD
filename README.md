# nRF_SWD
# Nordic_Channel_Sounding

## Table of contents
1. [About this project](#about)
    1. [Coding Philosophy](#philosophy)
    2. [How to make changes](#chage)
    3. [Comment your code](#comment)
2. [Guides](#guides)
    1. [Set-up for nRF52833 dk development](#subparagraph1)
3. [Project Sections](#project)
    1. [Reworking the distance measurement toolbox]()
    2. [Firmware deployment and prototyping]()
    3. [Interconnectivity and networking]()



## About the project <a name="about"></a>
### Coding Philosophy <a name="philosophy"></a>
- All changes will be peer-reviewed before entering main, a pull-request must be approved or commented on by another user before merging
- All changes must be well documented and commented before a peer-review
- Activly use the tools: Issues, Branches and Forks
- Readability counts: this repository uses CAPSLOCK for constants and peripheralname_name for all other applicable cases
- Small is beautiful: Prefer to make separate .c and .h files for each peripheral or major function
- nRF builds can become very large. With the exception of main/full_builds only upload the relevant files (usually .c, .h, prj.config)

### How to make changes <a name="change"></a>
There are three main ways to make changes to this repository
1. **Use Issues:** This is used for making suggestions, tasks or marking bugs. See more info under [Creating issues](https://docs.github.com/en/issues/tracking-your-work-with-issues/creating-an-issue). We recommend the method "Creating an issue from a repository". Use this when you have an idea/error but don't yet know how to fix it or wheter you want to fix it. You can think of an issue as a "mini-forum"
2. **Use Branches and Pull requests:** This is for when you think you have a solution to a problem done already. A branch is in essence a temporary workplace that you later can merge with the main repository. Once you have made changes to a branch you make a Pull request. The pull request lets others know which changes you have made to the branch, so that you can discuss and review before merging the content into main. (Try to avoid making branches of branches)
3. **Use Forks:** This is mostly used for major changes "when the intent is to create a independent project, which may never reunite". We strongly recommend that this is used when you want to change something fundamental in the project

### Comment your code <a name="comment"></a>
This project has three main types of comments: section comments, function comments and clarifying comments. All shown under:
```cpp
/***************************************************
            Section: Function definitions
****************************************************/


/**@brief Function compares size of integers
 * @param[in] int1 The first integer
 * @param[in] int2 The second integer
 * @return true if int1 is larger than int2
 */
int compare_integers(int int1, int int2){
    if(int1 > int2){ //Compares the two values
       return true;
   }
   else{
       return false;
   }
}

```

## Guides <a name="guides"></a>
### Set-up Guide <a name="setup"></a>
First of all related to "Coding Philosophy" **we recommend keeping your GitHub and nRF workspaces separate** during development. GitHub should strictly be used for core files, and it is easy to cause errors when they intermingle, even though GitHub easily can restore old settings this might be trouble. 

#### nRF Connect<a name="connect"></a>
To setup nRF Connect we recommend following the first two lessons of the [nRF Connect SDK Fundamentals course](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-1-nrf-connect-sdk-introduction/). Completing the entire course will give you more experience with the development tools, however this is not required.

#### GitHub on VS Code<a name="github"></a>
The following guide [Working with GitHub in VS Code](https://code.visualstudio.com/docs/sourcecontrol/github) completes a full workthrough of how to setup GitHub on your VS Code. However, given that GitHub is mostly used for core files it is fully possible to parttake in the project only using the Web-Interface. GitHub on VS Code especially makes cloning, creating issues and branches, and adding files simpler for the user. But we stress that all functionality is available in the Web-Interface
