$$
\begin{align}
[\text{Prog}] &\to [\text{Stmt}]^* \\
[\text{Stmt}] &\to
\begin{cases}
\text{ret}([\text{Expr}]); \\
\text{give}([\text{Expr}]); \\
\text{var}\space\text{ident} = [\text{Expr}]; \\
\text{ident} = \text{[Expr]}; \\
\text{ident}([\text{Expr}]^*); \\
\text{if} ([\text{Expr}])[\text{Scope}]\text{[IfPred]} \\
\text{while} ([\text{Expr}])[\text{Scope}] \\
\text{func}\space \text{ident}([\text{ident}]^*)[\text{Scope}]\\
[\text{Scope}]
\end{cases} \\

[\text{Expr}] &\to
\begin{cases}
[\text{Term}] \\
[\text{BinExpr}]
\end{cases} \\

[\text{BinExpr}] &\to
\begin{cases}
[\text{Expr}] * [\text{Expr}]  & \text{prec} = 1 \\
[\text{Expr}] / [\text{Expr}]  & \text{prec} = 1 \\
[\text{Expr}] + [\text{Expr}]  & \text{prec} = 0 \\
[\text{Expr}] - [\text{Expr}]  & \text{prec} = 0 \\
\\
[\text{Expr}] == [\text{Expr}]  & \text{prec} = -1 \\
[\text{Expr}] >= [\text{Expr}]  & \text{prec} = -1 \\
[\text{Expr}] <= [\text{Expr}]  & \text{prec} = -1 \\
[\text{Expr}] < [\text{Expr}]  & \text{prec} = -1 \\
[\text{Expr}] > [\text{Expr}]  & \text{prec} = -1 \\
\end{cases} \\

[\text{Term}] &\to
\begin{cases}
\text{int\_lit} \\
\text{ident} \\
\text{ident}([\text{Expr}]^*); \\
[\text{Expr}]
\end{cases} \\

[\text{Scope}] &\to [\text{Stmt}]^* \\

[\text{IfPred}] &\to
\begin{cases}
\text{wif}([\text{Expr}])[\text{Scope}][\text{IfPred}] \\
\text{else}[\text{Scope}] \\
\epsilon
\end{cases}

\end{align}
$$