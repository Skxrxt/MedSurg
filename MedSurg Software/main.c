#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define MAX_QUESTIONS 1000
#define MAX_CHOICES 5
#define MAX_TEXT 1000
#define MAX_QUIZZES 100
#define MAX_FILENAME 260


typedef struct {
    char type[20];
    char question[MAX_TEXT];
    char choices[MAX_CHOICES][MAX_TEXT];
    int choice_count;
    int correct_answer;
    int user_answer;
    int is_correct;
    char rationale[MAX_TEXT];
} Question;


/* ========================================
   REMOVE NEWLINE
   ======================================== */

void remove_newline(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}


/* ========================================
   CONVERT ANSWER LETTER TO NUMBER
   A = 0
   B = 1
   C = 2
   D = 3
   E = 4
   ======================================== */

int answer_to_index(char answer)
{
    answer = toupper(answer);

    if (answer >= 'A' && answer <= 'E') {
        return answer - 'A';
    }

    return -1;
}


/* ========================================
   FIND ALL QUIZ FILES
   ======================================== */

int find_quizzes(char quiz_files[][MAX_FILENAME], int max_quizzes)
{
    WIN32_FIND_DATAA file_data;
    HANDLE find_handle;

    int quiz_count = 0;

    find_handle = FindFirstFileA(
        "quizzes\\*.txt",
        &file_data
    );

    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {

        /* Ignore folders */
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            if (quiz_count < max_quizzes) {

                strcpy(
                    quiz_files[quiz_count],
                    file_data.cFileName
                );

                quiz_count++;
            }
        }

    } while (FindNextFileA(find_handle, &file_data));

    FindClose(find_handle);

    return quiz_count;
}


/* ========================================
   PARSE QUESTIONS FROM TXT FILE
   ======================================== */

int parse_questions(const char *filename, Question questions[])
{
    FILE *file;
    char line[MAX_TEXT];

    int question_count = 0;
    int in_question = 0;

    file = fopen(filename, "r");

    if (file == NULL) {

        printf("Error: Could not open %s\n", filename);

        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {

        remove_newline(line);


        /* ------------------------------
           START OF QUESTION
           ------------------------------ */

        if (strcmp(line, "[MCQ]") == 0) {

            if (question_count >= MAX_QUESTIONS) {

                printf("Maximum number of questions reached.\n");

                break;
            }

            strcpy(
                questions[question_count].type,
                "MCQ"
            );

            questions[question_count].choice_count = 0;

            questions[question_count].correct_answer = -1;

            questions[question_count].user_answer = -1;

            questions[question_count].is_correct = 0;

            questions[question_count].question[0] = '\0';

            questions[question_count].rationale[0] = '\0';

            in_question = 1;
        }


        /* ------------------------------
           QUESTION TEXT
           ------------------------------ */

        else if (
            strncmp(line, "QUESTION:", 9) == 0
            && in_question
        ) {

            strcpy(
                questions[question_count].question,
                line + 10
            );
        }


        /* ------------------------------
           ANSWER CHOICES A-E
           ------------------------------ */

        else if (
            in_question &&
            strlen(line) >= 2 &&
            line[1] == ':' &&
            line[0] >= 'A' &&
            line[0] <= 'E'
        ) {

            int index = line[0] - 'A';

            strcpy(
                questions[question_count].choices[index],
                line + 3
            );

            if (
                index >=
                questions[question_count].choice_count
            ) {

                questions[question_count].choice_count =
                    index + 1;
            }
        }


        /* ------------------------------
           CORRECT ANSWER
           ------------------------------ */

        else if (
            strncmp(line, "ANSWER:", 7) == 0
            && in_question
        ) {

            questions[question_count].correct_answer =
                answer_to_index(line[8]);
        }


        /* ------------------------------
           RATIONALE
           ------------------------------ */

        else if (
            strncmp(line, "RATIONALE:", 10) == 0
            && in_question
        ) {

            strcpy(
                questions[question_count].rationale,
                line + 11
            );
        }


        /* ------------------------------
           END OF QUESTION
           ------------------------------ */

        else if (
            strcmp(line, "END") == 0
            && in_question
        ) {

            question_count++;

            in_question = 0;
        }
    }

    fclose(file);

    return question_count;
}


/* ========================================
   SHUFFLE QUESTIONS
   ======================================== */

void shuffle_questions(int order[], int question_count)
{
    int i;

    /* Create original order */
    for (i = 0; i < question_count; i++) {

        order[i] = i;
    }


    /* Fisher-Yates shuffle */
    for (i = question_count - 1; i > 0; i--) {

        int j = rand() % (i + 1);

        int temp = order[i];

        order[i] = order[j];

        order[j] = temp;
    }
}


/* ========================================
   RUN QUIZ
   ======================================== */

void run_quiz(Question questions[], int question_count)
{
    int score = 0;

    int question_order[MAX_QUESTIONS];


    /* Randomize question order */
    shuffle_questions(
        question_order,
        question_count
    );


    /* ------------------------------------
       START QUIZ
       ------------------------------------ */

    for (int i = 0; i < question_count; i++) {

        int current = question_order[i];

        char answer;


        printf("\n========================================\n");

        printf(
            "Question %d of %d\n\n",
            i + 1,
            question_count
        );


        /* Question */

        printf(
            "%s\n\n",
            questions[current].question
        );


        /* Choices */

        for (
            int j = 0;
            j < questions[current].choice_count;
            j++
        ) {

            printf(
                "%c. %s\n",
                'A' + j,
                questions[current].choices[j]
            );
        }


        /* --------------------------------
           GET ANSWER
           -------------------------------- */

        while (1) {

            printf("\nYour answer (A-D): ");

            scanf(" %c", &answer);

            answer = toupper(answer);


            if (
                answer >= 'A'
                &&
                answer <= 'D'
            ) {

                break;
            }


            printf(
                "Invalid input. "
                "Please enter only A, B, C, or D.\n"
            );
        }


        questions[current].user_answer =
            answer_to_index(answer);


        /* --------------------------------
           CHECK ANSWER
           -------------------------------- */

        if (
            questions[current].user_answer
            ==
            questions[current].correct_answer
        ) {

            questions[current].is_correct = 1;

            score++;

            printf("\nCORRECT!\n");
        }

        else {

            questions[current].is_correct = 0;

            printf("\nWRONG\n");

            printf(
                "Correct answer: %c\n",
                'A' +
                questions[current].correct_answer
            );
        }


        /* --------------------------------
           SHOW RATIONALE
           -------------------------------- */

        printf("\nRationale:\n");

        printf(
            "%s\n",
            questions[current].rationale
        );


        /* --------------------------------
           WAIT FOR ENTER
           -------------------------------- */

        printf(
            "\nPress Enter to continue..."
        );

        getchar();
        getchar();


        /* --------------------------------
           STOP OPTION EVERY 10 QUESTIONS
           -------------------------------- */

        if (
            (i + 1) % 10 == 0
            &&
            i + 1 < question_count
        ) {

            int choice;


            printf(
                "\n========================================\n"
            );

            printf(
                "10 QUESTIONS COMPLETED\n"
            );

            printf(
                "========================================\n"
            );


            printf(
                "Current score: %d / %d\n",
                score,
                i + 1
            );


            printf("\n1. Continue\n");

            printf("2. Stop Quiz\n");


            /* Get valid menu choice */

            while (1) {

                printf("\nSelect: ");

                scanf("%d", &choice);


                if (
                    choice == 1
                    ||
                    choice == 2
                ) {

                    break;
                }


                printf(
                    "Invalid choice. "
                    "Please enter 1 or 2.\n"
                );
            }


            /* Stop quiz */

            if (choice == 2) {

                printf(
                    "\n========================================\n"
                );

                printf(
                    "QUIZ STOPPED\n"
                );

                printf(
                    "========================================\n"
                );


                printf(
                    "Score so far: %d / %d\n",
                    score,
                    i + 1
                );


                return;
            }
        }
    }


    /* ------------------------------------
       QUIZ COMPLETE
       ------------------------------------ */

    printf(
        "\n========================================\n"
    );

    printf(
        "QUIZ COMPLETE\n"
    );

    printf(
        "========================================\n"
    );


    printf(
        "Score: %d / %d\n",
        score,
        question_count
    );


    printf(
        "Percentage: %.2f%%\n",
        (float)score /
        question_count *
        100
    );
}


/* ========================================
   MAIN
   ======================================== */

int main()
{
    char quiz_files[MAX_QUIZZES][MAX_FILENAME];

    int quiz_count;

    int choice;

    static Question questions[MAX_QUESTIONS];
    int question_count;

    char filepath[MAX_FILENAME + 20];


    /* ------------------------------------
       START RANDOM NUMBER GENERATOR
       ------------------------------------ */

    srand(
        (unsigned int)time(NULL)
    );


    /* ------------------------------------
       FIND QUIZZES
       ------------------------------------ */

    quiz_count =
        find_quizzes(
            quiz_files,
            MAX_QUIZZES
        );


    if (quiz_count == 0) {

        printf(
            "\nNo quiz files found.\n"
        );

        printf(
            "Make sure your quiz files are inside:\n"
        );

        printf(
            "quizzes\\\n"
        );

        printf(
            "\nPress Enter to exit..."
        );

        getchar();

        return 1;
    }


    /* ------------------------------------
       DISPLAY QUIZ MENU
       ------------------------------------ */

    printf(
        "\n========================================\n"
    );

    printf(
        "       NURSING QUIZ PROGRAM\n"
    );

    printf(
        "========================================\n"
    );


    printf(
        "\nAvailable Quizzes:\n\n"
    );


    for (int i = 0; i < quiz_count; i++) {

        printf(
            "%d. %s\n",
            i + 1,
            quiz_files[i]
        );
    }


    printf("\n0. Exit\n");


    /* ------------------------------------
       SELECT QUIZ
       ------------------------------------ */

    while (1) {

        printf(
            "\nSelect a quiz: "
        );

        scanf(
            "%d",
            &choice
        );


        if (
            choice >= 0
            &&
            choice <= quiz_count
        ) {

            break;
        }


        printf(
            "Invalid choice. "
            "Please select a valid number.\n"
        );
    }


    /* Exit */

    if (choice == 0) {

        printf(
            "\nGoodbye!\n"
        );

        return 0;
    }


    /* ------------------------------------
       CREATE FILE PATH
       ------------------------------------ */

    snprintf(
        filepath,
        sizeof(filepath),
        "quizzes\\%s",
        quiz_files[choice - 1]
    );


    /* ------------------------------------
       LOAD QUESTIONS
       ------------------------------------ */

    question_count =
        parse_questions(
            filepath,
            questions
        );


    if (question_count <= 0) {

        printf(
            "\nNo questions could be loaded.\n"
        );

        printf(
            "\nPress Enter to exit..."
        );

        getchar();
        getchar();

        return 1;
    }


    /* ------------------------------------
       SHOW LOADED QUESTION COUNT
       ------------------------------------ */

    printf(
        "\nLoaded %d questions.\n",
        question_count
    );


    printf(
        "\nPress Enter to start..."
    );

    getchar();
    getchar();


    /* ------------------------------------
       RUN QUIZ
       ------------------------------------ */

    run_quiz(
        questions,
        question_count
    );


    /* ------------------------------------
       END
       ------------------------------------ */

    printf(
        "\nPress Enter to return to Windows..."
    );

    getchar();
    getchar();


    return 0;
}