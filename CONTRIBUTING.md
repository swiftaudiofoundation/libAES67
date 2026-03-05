# Contributing to libAES67

Thank you for your interest in contributing to libAES67! We appreciate all kinds of contributions: bug reports, feature requests, documentation, and code improvements.

## How to Contribute

1. **Fork the repository** and clone it locally.
2. **Create a new branch** for your changes:
   ```
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes** with clear and meaningful commit messages. Follow the commit message template for consistency.
4. **Run tests and code style checks** to ensure quality.
5. **Push your branch** to your fork:
   ```
   git push origin feature/your-feature-name
   ```
6. **Open a Pull Request** against the main repository describing your changes and motivation.

## Commit Message Guidelines

Use the provided commit message template to ensure your commit messages are structured and informative. Include:

- A concise summary of your change.
- Motivation for the change.
- Description of modifications.
- The result or impact.

Example:
```
Add feature X to improve Y

### Motivation

This feature improves Y by ...

### Modifications

- Added class Foo
- Updated method Bar to handle ...

### Result

- Better performance
- Fixes issue #123
```

## Code Style

Maintaining a consistent code style is crucial for readability and maintainability.

Please use the provided script to check and fix code style issues:

```bash
./scripts/codestyle.sh
```

Run this before submitting your changes to avoid formatting issues.

### Manual style notes:
- Follow existing indentation and spacing conventions.
- Keep lines within 80-100 characters if possible.
- Use meaningful variable and function names.

## Testing

- Run all existing unit tests to ensure nothing is broken.
- Add new tests to cover your changes where applicable.
- Verify that no warnings or errors are introduced.

## Reporting Issues

If you find bugs or have feature requests, please use the issue tracker.
Provide as much detail as possible, including steps to reproduce and expected vs actual behavior.
