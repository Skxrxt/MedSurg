#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#define MAX_QUESTIONS 100
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


/* Remove newline at the end of a string */
void remove_newline(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}


/* Convert A/B/C/D/E into 0/1/2/3/4 */
int answer_to_index(char answer)
{
    answer = toupper(answer);

    if (answer >= 'A' && answer <= 'E') {
        return answer - 'A';
    }

    return -1;
}

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

        /* Start of a question */
        if (strcmp(line, "[MCQ]") == 0) {

            if (question_count >= MAX_QUESTIONS) {
                printf("Maximum number of questions reached.\n");
                break;
            }

            strcpy(questions[question_count].type, "MCQ");

            questions[question_count].choice_count = 0;
            questions[question_count].correct_answer = -1;
            questions[question_count].question[0] = '\0';
            questions[question_count].rationale[0] = '\0';

            in_question = 1;
        }

        /* Question text */
        else if (strncmp(line, "QUESTION:", 9) == 0 && in_question) {

            strcpy(
                questions[question_count].question,
                line + 10
            );
        }

        /* Choices A-E */
        else if (in_question &&
                 strlen(line) >= 2 &&
                 line[1] == ':' &&
                 line[0] >= 'A' &&
                 line[0] <= 'E') {

            int index = line[0] - 'A';

            strcpy(
                questions[question_count].choices[index],
                line + 3
            );

            if (index >= questions[question_count].choice_count) {
                questions[question_count].choice_count = index + 1;
            }
        }

        /* Correct answer */
        else if (strncmp(line, "ANSWER:", 7) == 0 && in_question) {

            questions[question_count].correct_answer =
                answer_to_index(line[8]);
        }

        /* Rationale */
        else if (strncmp(line, "RATIONALE:", 10) == 0 && in_question) {

            strcpy(
                questions[question_count].rationale,
                line + 11
            );
        }

        /* End of question */
        else if (strcmp(line, "END") == 0 && in_question) {

            question_count++;
            in_question = 0;
        }
    }

    fclose(file);

    return question_count;
}

void run_quiz(Question questions[], int question_count)
{
    int score = 0;

    for (int i = 0; i < question_count; i++) {

        char answer;

        printf("\n========================================\n");
        printf("Question %d of %d\n\n", i + 1, question_count);

        printf("%s\n\n", questions[i].question);

        /* Display choices */
        for (int j = 0; j < questions[i].choice_count; j++) {

            printf("%c. %s\n",
                   'A' + j,
                   questions[i].choices[j]);
        }

        /* Get user's answer */
while (1) {

    printf("\nYour answer (A-D): ");
    scanf(" %c", &answer);

    answer = toupper(answer);

    if (answer >= 'A' && answer <= 'D') {
        break;
    }

    printf("Invalid input. Please enter only A, B, C, or D.\n");
}

questions[i].user_answer = answer_to_index(answer);
        /* Show rationale */
        printf("\nRationale:\n");
        printf("%s\n", questions[i].rationale);

        printf("\nPress Enter to continue...");

        getchar();  // consume leftover newline
        getchar();  // wait for Enter
    }

    /* Final score */
    printf("\n========================================\n");
    printf("QUIZ COMPLETE\n");
    printf("========================================\n");

    printf("Score: %d / %d\n",
           score,
           question_count);

    printf("Percentage: %.2f%%\n",
           (float)score / question_count * 100);
}

int main()
{
    Question questions[MAX_QUESTIONS];

    char quiz_files[MAX_QUIZZES][MAX_FILENAME];

    int quiz_count;
    int choice;
    int question_count;

    while (1) {

        /* Find all quiz files */
        quiz_count = find_quizzes(
            quiz_files,
            MAX_QUIZZES
        );

        if (quiz_count == 0) {

            printf("No quiz files found.\n");
            printf("Make sure your .txt files are inside the quizzes folder.\n");

            return 1;
        }

        /* Display quiz menu */
        printf("\n========================================\n");
        printf("        NURSING PRACTICE QUIZ\n");
        printf("========================================\n\n");

        printf("Available Quizzes:\n\n");

        for (int i = 0; i < quiz_count; i++) {

            printf("%d. %s\n",
                   i + 1,
                   quiz_files[i]);
        }

        printf("\n0. Exit\n");

        printf("\nSelect a quiz: ");
        scanf("%d", &choice);

        /* Exit program */
        if (choice == 0) {
            break;
        }

        /* Invalid choice */
        if (choice < 1 || choice > quiz_count) {

            printf("\nInvalid selection.\n");
            continue;
        }

        /* Build path */
        char filepath[MAX_FILENAME + 20];

        snprintf(
            filepath,
            sizeof(filepath),
            "quizzes\\%s",
            quiz_files[choice - 1]
        );

        /* Load quiz */
        question_count = parse_questions(
            filepath,
            questions
        );

        if (question_count < 0) {
            continue;
        }

        printf("\nLoaded %d questions.\n",
               question_count);

        /* Run quiz */
        run_quiz(
            questions,
            question_count
        );

        /* After quiz */
        printf("\n========================================\n");
        printf("              QUIZ MENU\n");
        printf("========================================\n\n");

        printf("1. Restart this quiz\n");
        printf("2. Choose another quiz\n");
        printf("0. Exit\n");

        printf("\nSelect: ");
        scanf("%d", &choice);

        /* Restart same quiz */
        if (choice == 1) {

            /* Run the same quiz again */
            run_quiz(
                questions,
                question_count
            );
        }

        /* Choose another quiz */
        else if (choice == 2) {

            continue;
        }

        /* Exit */
        else if (choice == 0) {

            break;
        }

        /* Invalid choice */
        else {

            printf("\nInvalid selection. Returning to quiz menu...\n");
        }
    }

    printf("\nGoodbye!\n");

    return 0;
}