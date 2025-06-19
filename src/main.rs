use std::io::{self, Write};
use std::process;

fn tokenize(input: &str) -> Vec<String> {
    let mut tokens = Vec::new();
    
    let mut token = String::new();
    
    let mut in_quotes = false;
    for c in input.chars() {
        match c {
            '\'' => {
                in_quotes = !in_quotes;
            }
            c if c.is_whitespace() && !in_quotes => {
                if !token.is_empty() {
                    tokens.push(token);
                    
                    token = String::new();
                }
            }
            _ => {
                token.push(c);
            }
        }
    }

    if !token.is_empty() {
        tokens.push(token);
    }

    tokens
}

fn main() {
    
    loop {
        
        print!("rsh> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        let cargv = tokenize(input.trim());

        if cargv[0] == "exit" {
            process::exit(cargv[1].parse::<i32>().unwrap());
        } else if cargv[0] == "echo" {
            for arg in cargv.iter().skip(1) {
                print!("{} ", arg);
            }
            println!();
        } else if cargv.is_empty() {
            continue;
        } else {
            println!("{} command not found", cargv[0]);
        }
    }
}