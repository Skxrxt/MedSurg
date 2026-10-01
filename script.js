let questions = [];
let currentQuestion = 0;
let score = 0;

let selectedQuiz = "";
let selectedQuizName = "";


// ========================================
// GITHUB QUIZ FOLDER
// ========================================

const GITHUB_API_URL =
    "https://api.github.com/repos/Skxrxt/MedSurg/contents/quizzes";


// ========================================
// LOAD QUIZ LIST FROM GITHUB
// ========================================

async function loadQuizList() {

    const quizList =
        document.getElementById("quizList");

    quizList.innerHTML =
        "<p>Loading quizzes...</p>";

    try {

        const response =
            await fetch(GITHUB_API_URL, {
                headers: {
                    "Accept":
                        "application/vnd.github+json"
                }
            });


        if (!response.ok) {

            throw new Error(
                "Could not load quizzes from GitHub."
            );

        }


        const files =
            await response.json();


        // ====================================
        // ONLY USE .TXT FILES
        // ====================================

        const quizzes =
            files.filter(file =>
                file.type === "file" &&
                file.name
                    .toLowerCase()
                    .endsWith(".txt")
            );


        quizList.innerHTML = "";


        // ====================================
        // NO QUIZZES FOUND
        // ====================================

        if (quizzes.length === 0) {

            quizList.innerHTML = `

                <div class="error-message">

                    <strong>
                        No quizzes found.
                    </strong>

                    <p>
                        There are no .txt quiz files
                        in the GitHub quizzes folder.
                    </p>

                </div>

            `;

            return;
        }


        // ====================================
        // SORT QUIZZES ALPHABETICALLY
        // ====================================

        quizzes.sort((a, b) =>
            a.name.localeCompare(b.name)
        );


        // ====================================
        // CREATE QUIZ BUTTONS
        // ====================================

        quizzes.forEach((quiz) => {

            const button =
                document.createElement("button");


            button.className =
                "quiz-button";


            button.textContent =
                formatQuizName(quiz.name);


            button.addEventListener(
                "click",
                () => {

                    startQuiz(
                        quiz.download_url,
                        formatQuizName(quiz.name)
                    );

                }
            );


            quizList.appendChild(button);

        });


    } catch (error) {

        console.error(error);


        quizList.innerHTML = `

            <div class="error-message">

                <strong>
                    Could not load quizzes.
                </strong>

                <p>
                    The quiz list could not be
                    retrieved from GitHub.
                </p>

                <button
                    id="retryQuizListButton"
                    class="back-button"
                >
                    Try Again
                </button>

            </div>

        `;


        const retryButton =
            document.getElementById(
                "retryQuizListButton"
            );


        if (retryButton) {

            retryButton.addEventListener(
                "click",
                () => {

                    loadQuizList();

                }
            );

        }

    }
}


// ========================================
// FORMAT QUIZ NAME
// ========================================

function formatQuizName(filename) {

    // Remove .txt
    let name =
        filename.replace(/\.txt$/i, "");


    // Replace _ and - with spaces
    name =
        name.replace(/[_-]+/g, " ");


    // Capitalize each word
    name =
        name.replace(
            /\b\w/g,
            letter => letter.toUpperCase()
        );


    return name;
}


// ========================================
// START QUIZ
// ========================================

function startQuiz(filename, name) {

    selectedQuiz = filename;
    selectedQuizName = name;

    questions = [];

    currentQuestion = 0;

    score = 0;


    // ====================================
    // SHOW QUIZ SCREEN
    // ====================================

    document.getElementById("menu")
        .style.display = "none";


    document.getElementById("quizScreen")
        .style.display = "block";


    // ====================================
    // HIDE OTHER SCREENS
    // ====================================

    document.getElementById("checkpoint")
        .style.display = "none";


    document.getElementById("stoppedScreen")
        .style.display = "none";


    document.getElementById("completeScreen")
        .style.display = "none";


    // ====================================
    // QUIZ TITLE
    // ====================================

    document.getElementById("quizTitle")
        .textContent = name;


    // ====================================
    // RESET BUTTONS
    // ====================================

    document.getElementById("submitButton")
        .style.display = "block";


    document.getElementById("nextButton")
        .style.display = "none";


    document.getElementById("result")
        .innerHTML = "";


    document.getElementById("question")
        .textContent = "Loading quiz...";


    document.getElementById("choices")
        .innerHTML = "";


    document.getElementById("questionNumber")
        .textContent = "";


    // ====================================
    // LOAD QUIZ
    // ====================================

    loadQuiz(filename);
}


// ========================================
// LOAD QUIZ TXT FROM GITHUB
// ========================================

async function loadQuiz(downloadURL) {

    try {

        const response =
            await fetch(downloadURL);


        if (!response.ok) {

            throw new Error(
                "Quiz file could not be loaded."
            );

        }


        const text =
            await response.text();


        // ====================================
        // PARSE QUESTIONS
        // ====================================

        parseQuestions(text);


        // ====================================
        // CHECK IF QUESTIONS EXIST
        // ====================================

        if (questions.length === 0) {

            throw new Error(
                "No valid questions were found."
            );

        }


        // ====================================
        // RANDOMIZE QUESTIONS
        // ====================================

        shuffleQuestions();


        // ====================================
        // DISPLAY FIRST QUESTION
        // ====================================

        displayQuestion();


    } catch (error) {

        console.error(error);


        showQuizError();

    }
}


// ========================================
// SHOW QUIZ ERROR
// ========================================

function showQuizError() {

    document.getElementById("question")
        .innerHTML = `

            <div class="error-message">

                <strong>
                    Unable to load this quiz.
                </strong>

                <p>
                    The quiz file could not be
                    found or could not be read.
                </p>

                <button
                    id="errorBackButton"
                    class="back-button"
                >
                    ← Go Back
                </button>

            </div>

        `;


    document.getElementById("choices")
        .innerHTML = "";


    document.getElementById("questionNumber")
        .textContent = "";


    document.getElementById("submitButton")
        .style.display = "none";


    document.getElementById("nextButton")
        .style.display = "none";


    document.getElementById("result")
        .innerHTML = "";


    document.getElementById("errorBackButton")
        .addEventListener(
            "click",
            () => {

                returnToMenu();

            }
        );
}


// ========================================
// PARSE QUESTIONS
// ========================================

function parseQuestions(text) {

    questions = [];


    const lines =
        text
            .replace(/\r/g, "")
            .split("\n");


    let current = null;


    for (let line of lines) {

        line = line.trim();


        // ====================================
        // MCQ START
        // ====================================

        if (line === "[MCQ]") {

            current = {

                type: "MCQ",

                question: "",

                choices: [],

                choice_count: 0,

                correct_answer: -1,

                user_answer: -1,

                is_correct: false,

                rationale: ""

            };


            continue;
        }


        // Ignore lines outside a question
        if (!current) {

            continue;

        }


        // ====================================
        // QUESTION
        // ====================================

        if (line.startsWith("QUESTION:")) {

            current.question =
                line
                    .substring("QUESTION:".length)
                    .trim();


            continue;
        }


        // ====================================
        // ANSWER CHOICES
        // ====================================

        if (/^[A-D]:/.test(line)) {

            const choice =
                line.substring(2).trim();


            current.choices.push(choice);


            current.choice_count++;


            continue;
        }


        // ====================================
        // CORRECT ANSWER
        // ====================================

        if (line.startsWith("ANSWER:")) {

            const answer =
                line
                    .substring("ANSWER:".length)
                    .trim()
                    .toUpperCase();


            current.correct_answer =
                answerToIndex(answer);


            continue;
        }


        // ====================================
        // RATIONALE
        // ====================================

        if (line.startsWith("RATIONALE:")) {

            current.rationale =
                line
                    .substring("RATIONALE:".length)
                    .trim();


            continue;
        }


        // ====================================
        // END QUESTION
        // ====================================

        if (line === "END") {

            // Only add valid questions
            if (
                current.question !== "" &&
                current.choices.length >= 2 &&
                current.correct_answer >= 0
            ) {

                questions.push(current);

            }


            current = null;


            continue;
        }

    }
}


// ========================================
// CONVERT A-D TO INDEX
// ========================================

function answerToIndex(answer) {

    if (answer === "A") return 0;

    if (answer === "B") return 1;

    if (answer === "C") return 2;

    if (answer === "D") return 3;


    return -1;
}


// ========================================
// SHUFFLE QUESTIONS
// ========================================

function shuffleQuestions() {

    for (
        let i = questions.length - 1;
        i > 0;
        i--
    ) {

        const j =
            Math.floor(
                Math.random() * (i + 1)
            );


        [
            questions[i],
            questions[j]
        ] =
        [
            questions[j],
            questions[i]
        ];

    }
}


// ========================================
// DISPLAY QUESTION
// ========================================

function displayQuestion() {

    const q =
        questions[currentQuestion];


    // ====================================
    // QUESTION NUMBER
    // ====================================

    document.getElementById("questionNumber")
        .textContent =
            `Question ${
                currentQuestion + 1
            } of ${
                questions.length
            }`;


    // ====================================
    // QUESTION TEXT
    // ====================================

    document.getElementById("question")
        .textContent =
            q.question;


    // ====================================
    // CLEAR OLD CHOICES
    // ====================================

    const choicesContainer =
        document.getElementById("choices");


    choicesContainer.innerHTML = "";


    // ====================================
    // CREATE ANSWER CHOICES
    // ====================================

    q.choices.forEach(
        (choice, index) => {

            const label =
                document.createElement("label");


            label.className =
                "answer-option";


            label.innerHTML = `

                <input
                    type="radio"
                    name="answer"
                    value="${index}"
                >

                <span>
                    ${
                        String.fromCharCode(
                            65 + index
                        )
                    }. ${choice}
                </span>

            `;


            choicesContainer.appendChild(label);

        }
    );


    // ====================================
    // RESET RESULT
    // ====================================

    document.getElementById("result")
        .innerHTML = "";


    // ====================================
    // SHOW SUBMIT
    // ====================================

    document.getElementById("submitButton")
        .style.display = "block";


    // ====================================
    // HIDE NEXT
    // ====================================

    document.getElementById("nextButton")
        .style.display = "none";
}


// ========================================
// SUBMIT ANSWER
// ========================================

document.getElementById("submitButton")
    .addEventListener(
        "click",
        () => {

            const selected =
                document.querySelector(
                    'input[name="answer"]:checked'
                );


            // ====================================
            // NO ANSWER SELECTED
            // ====================================

            if (!selected) {

                document.getElementById("result")
                    .innerHTML = `
                        <p>
                            Please select an answer.
                        </p>
                    `;

                return;

            }


            const userAnswer =
                parseInt(selected.value);


            const q =
                questions[currentQuestion];


            q.user_answer =
                userAnswer;


            q.is_correct =
                userAnswer === q.correct_answer;


            // ====================================
            // CORRECT
            // ====================================

            if (q.is_correct) {

                score++;


                document.getElementById("result")
                    .innerHTML = `

                        <div class="correct">

                            <strong>
                                Correct!
                            </strong>

                            <p>
                                ${q.rationale}
                            </p>

                        </div>

                    `;

            }


            // ====================================
            // INCORRECT
            // ====================================

            else {

                document.getElementById("result")
                    .innerHTML = `

                        <div class="wrong">

                            <strong>
                                Incorrect.
                            </strong>

                            <p>
                                Correct answer:
                                ${
                                    String.fromCharCode(
                                        65 +
                                        q.correct_answer
                                    )
                                }
                            </p>

                            <p>
                                ${q.rationale}
                            </p>

                        </div>

                    `;

            }


            // ====================================
            // DISABLE ANSWERS
            // ====================================

            document.querySelectorAll(
                'input[name="answer"]'
            ).forEach(input => {

                input.disabled = true;

            });


            // ====================================
            // HIDE SUBMIT
            // ====================================

            document.getElementById("submitButton")
                .style.display = "none";


            // ====================================
            // SHOW NEXT
            // ====================================

            document.getElementById("nextButton")
                .style.display = "block";

        }
    );


// ========================================
// NEXT QUESTION
// ========================================

document.getElementById("nextButton")
    .addEventListener(
        "click",
        () => {

            currentQuestion++;


            // ====================================
            // CHECKPOINT EVERY 10 QUESTIONS
            // ====================================

            if (
                currentQuestion > 0 &&
                currentQuestion % 10 === 0 &&
                currentQuestion < questions.length
            ) {

                // Show current score
                document.getElementById(
                    "checkpointScore"
                ).textContent =
                    `Score: ${
                        score
                    } / ${
                        currentQuestion
                    }`;


                // Hide quiz
                document.getElementById("quizScreen")
                    .style.display = "none";


                // Show checkpoint
                document.getElementById("checkpoint")
                    .style.display = "block";


                return;

            }


            // ====================================
            // QUIZ FINISHED
            // ====================================

            if (
                currentQuestion >=
                questions.length
            ) {

                finishQuiz();

                return;

            }


            // ====================================
            // NEXT QUESTION
            // ====================================

            displayQuestion();

        }
    );


// ========================================
// CONTINUE AFTER CHECKPOINT
// ========================================

document.getElementById("continueButton")
    .addEventListener(
        "click",
        () => {

            // Hide checkpoint
            document.getElementById("checkpoint")
                .style.display = "none";


            // Show quiz
            document.getElementById("quizScreen")
                .style.display = "block";


            // Display next question
            displayQuestion();

        }
    );


// ========================================
// STOP QUIZ
// ========================================

document.getElementById("stopButton")
    .addEventListener(
        "click",
        () => {

            // Hide checkpoint
            document.getElementById("checkpoint")
                .style.display = "none";


            // Hide quiz
            document.getElementById("quizScreen")
                .style.display = "none";


            // Show stopped screen
            document.getElementById("stoppedScreen")
                .style.display = "block";


            // Show score
            document.getElementById("stoppedScore")
                .textContent =
                    `Score: ${
                        score
                    } / ${
                        currentQuestion
                    }`;

        }
    );


// ========================================
// FINISH QUIZ
// ========================================

function finishQuiz() {

    // Hide quiz
    document.getElementById("quizScreen")
        .style.display = "none";


    // Show complete screen
    document.getElementById("completeScreen")
        .style.display = "block";


    // Calculate percentage
    const percentage =
        Math.round(
            (score / questions.length) * 100
        );


    // Final score
    document.getElementById("finalScore")
        .textContent =
            `Score: ${
                score
            } / ${
                questions.length
            }`;


    // Final percentage
    document.getElementById("finalPercentage")
        .textContent =
            `${percentage}%`;

}


// ========================================
// RETAKE QUIZ
// ========================================

document.getElementById("retakeButton")
    .addEventListener(
        "click",
        () => {

            startQuiz(
                selectedQuiz,
                selectedQuizName
            );

        }
    );


// ========================================
// RETURN TO MENU
// ========================================

function returnToMenu() {

    // Reset quiz data
    questions = [];

    currentQuestion = 0;

    score = 0;

    selectedQuiz = "";

    selectedQuizName = "";


    // Hide quiz
    document.getElementById("quizScreen")
        .style.display = "none";


    // Hide checkpoint
    document.getElementById("checkpoint")
        .style.display = "none";


    // Hide stopped screen
    document.getElementById("stoppedScreen")
        .style.display = "none";


    // Hide complete screen
    document.getElementById("completeScreen")
        .style.display = "none";


    // Show menu
    document.getElementById("menu")
        .style.display = "block";


    // Reload quiz list from GitHub
    loadQuizList();

}


// ========================================
// RETURN TO MENU FROM COMPLETION
// ========================================

document.getElementById("completeMenuButton")
    .addEventListener(
        "click",
        () => {

            returnToMenu();

        }
    );


// ========================================
// RETURN TO MENU FROM STOPPED SCREEN
// ========================================

document.getElementById("returnMenuButton")
    .addEventListener(
        "click",
        () => {

            returnToMenu();

        }
    );


// ========================================
// INITIAL LOAD
// ========================================

loadQuizList();
